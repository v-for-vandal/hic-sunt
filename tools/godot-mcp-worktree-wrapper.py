#!/usr/bin/env python3
"""Worktree-aware launcher for NPGameDev Godot MCP Toolkit.

The official MCP bridge discovers an editor instance from process.cwd() or
GODOT_MCP_PROJECT_PATH. In this repository the Godot project is a subdirectory
of each git worktree, so this wrapper computes the correct project path before
starting the bridge.

It also starts a headless Godot editor only when the toolkit registry does not
already contain a live editor for this exact absolute project path. Editors that
were already running (GUI or headless) are reused and never terminated by this
wrapper. A headless editor started by this wrapper is terminated when the MCP
bridge exits.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import os
import signal
import socket
import subprocess
import sys
import time
from pathlib import Path
from typing import Any

DEFAULT_GODOT_SUBDIR = Path("ui/godot/project")
REGISTRY_RELATIVE = Path("godot-mcp-toolkit/projects.json")
REGISTRY_TIMEOUT_SECONDS = 45.0
POLL_SECONDS = 0.25


def log(message: str) -> None:
    print(f"[godot-mcp-worktree] {message}", file=sys.stderr, flush=True)


def run_text(args: list[str], cwd: Path) -> str:
    return subprocess.check_output(args, cwd=str(cwd), text=True, stderr=subprocess.DEVNULL).strip()


def git_worktree_root(start: Path) -> Path:
    try:
        return Path(run_text(["git", "rev-parse", "--show-toplevel"], start)).resolve()
    except Exception:
        # Fall back to walking upward so --status still gives useful diagnostics
        # when invoked from an unusual cwd.
        for parent in [start.resolve(), *start.resolve().parents]:
            if (parent / ".git").exists():
                return parent
        raise SystemExit("Unable to determine git worktree root")


def candidate_project_roots(worktree: Path) -> list[Path]:
    configured = os.environ.get("GODOT_MCP_PROJECT_SUBDIR")
    candidates: list[Path] = []
    if configured:
        candidates.append((worktree / configured).resolve())
    candidates.append((worktree / DEFAULT_GODOT_SUBDIR).resolve())

    # Fallback discovery. Ignore vendored/test projects and content subprojects;
    # prefer the project containing the toolkit addon.
    for project_file in worktree.rglob("project.godot"):
        rel = project_file.relative_to(worktree)
        parts = set(rel.parts)
        if "submodules" in parts or "subprojects" in parts:
            continue
        if rel.parts[:4] == ("ui", "godot", "content", "worldgen-basic"):
            continue
        candidates.append(project_file.parent.resolve())

    seen: set[Path] = set()
    unique: list[Path] = []
    for path in candidates:
        if path not in seen and (path / "project.godot").is_file():
            seen.add(path)
            unique.append(path)

    unique.sort(key=lambda p: 0 if (p / "addons/godot_mcp_toolkit/plugin.cfg").exists() else 1)
    return unique


def godot_project_root(worktree: Path) -> Path:
    candidates = candidate_project_roots(worktree)
    if not candidates:
        raise SystemExit(f"No project.godot found below {worktree}")
    return candidates[0]


def registry_path() -> Path:
    if sys.platform.startswith("win"):
        base = Path(os.environ.get("APPDATA", str(Path.home() / "AppData/Roaming")))
    elif sys.platform == "darwin":
        base = Path.home() / "Library/Application Support"
    else:
        base = Path(os.environ.get("XDG_DATA_HOME", str(Path.home() / ".local/share")))
    return base / REGISTRY_RELATIVE


def read_registry() -> dict[str, Any]:
    path = registry_path()
    try:
        with path.open("r", encoding="utf-8") as fh:
            data = json.load(fh)
            if isinstance(data, dict):
                return data
    except FileNotFoundError:
        return {}
    except json.JSONDecodeError as exc:
        log(f"Registry JSON parse failed: {path}: {exc}")
    return {}


def entry_for(project: Path) -> dict[str, Any] | None:
    by_path = read_registry().get("by_path", {})
    if not isinstance(by_path, dict):
        return None
    entry = by_path.get(str(project))
    return entry if isinstance(entry, dict) else None


def proc_cmdline(pid: int) -> list[str] | None:
    try:
        raw = Path(f"/proc/{pid}/cmdline").read_bytes()
        if not raw:
            return []
        return [part.decode("utf-8", "replace") for part in raw.split(b"\0") if part]
    except FileNotFoundError:
        return None
    except PermissionError:
        try:
            os.kill(pid, 0)
            return []
        except OSError:
            return None


def process_alive(pid: int) -> bool:
    if pid <= 0:
        return False
    try:
        os.kill(pid, 0)
        return True
    except OSError:
        return False


def port_open(port: int, host: str = "127.0.0.1", timeout: float = 0.35) -> bool:
    try:
        with socket.create_connection((host, port), timeout=timeout):
            return True
    except OSError:
        return False


def cmdline_project_path(cmdline: list[str]) -> Path | None:
    for index, arg in enumerate(cmdline):
        if arg == "--path" and index + 1 < len(cmdline):
            return Path(cmdline[index + 1]).expanduser().resolve()
        if arg.startswith("--path="):
            return Path(arg.split("=", 1)[1]).expanduser().resolve()
    return None


def entry_is_live_for_project(project: Path, entry: dict[str, Any] | None) -> bool:
    if not entry:
        return False
    try:
        pid = int(entry.get("pid"))
        port = int(entry.get("port"))
    except (TypeError, ValueError):
        return False

    if not process_alive(pid):
        return False

    cmdline = proc_cmdline(pid)
    if cmdline is not None:
        joined = "\0".join(cmdline)
        if "godot" not in joined.lower():
            return False
        cmd_project = cmdline_project_path(cmdline)
        if cmd_project is not None and cmd_project != project:
            # Registry is path-keyed, and this verifies a visible --path did not
            # point elsewhere. If Godot was launched from a project manager and
            # no --path is visible, rely on registry key + live pid + open port.
            return False

    return port_open(port)


def project_hash(project: Path) -> str:
    return hashlib.sha256(str(project).encode("utf-8")).hexdigest()[:16]


def runtime_dir(project: Path) -> Path:
    base = Path(os.environ.get("XDG_RUNTIME_DIR", "/tmp")) / "godot-mcp-worktree-wrapper"
    path = base / project_hash(project)
    path.mkdir(parents=True, exist_ok=True)
    return path


def find_godot_binary() -> str:
    return os.environ.get("GODOT_BIN", "godot")


def start_headless_editor(project: Path) -> subprocess.Popen[Any]:
    log_path = runtime_dir(project) / "godot-headless.log"
    log_file = log_path.open("ab", buffering=0)
    args = [find_godot_binary(), "--headless", "--editor", "--path", str(project)]
    log(f"Starting headless Godot editor: {' '.join(args)}")
    log(f"Godot log: {log_path}")
    return subprocess.Popen(
        args,
        cwd=str(project),
        stdin=subprocess.DEVNULL,
        stdout=log_file,
        stderr=subprocess.STDOUT,
        start_new_session=True,
    )


def wait_for_registry(project: Path, started: subprocess.Popen[Any] | None) -> dict[str, Any]:
    deadline = time.monotonic() + REGISTRY_TIMEOUT_SECONDS
    while time.monotonic() < deadline:
        if started is not None and started.poll() is not None:
            raise SystemExit(f"Headless Godot exited early with code {started.returncode}")
        entry = entry_for(project)
        if entry_is_live_for_project(project, entry):
            return entry or {}
        time.sleep(POLL_SECONDS)
    raise SystemExit(
        f"Timed out waiting for Godot MCP registry entry for {project}. "
        f"Registry: {registry_path()}"
    )


def terminate_started(process: subprocess.Popen[Any] | None) -> None:
    if process is None or process.poll() is not None:
        return
    log(f"Terminating self-started headless Godot pid={process.pid}")
    try:
        os.killpg(process.pid, signal.SIGTERM)
    except OSError:
        process.terminate()
    try:
        process.wait(timeout=8)
    except subprocess.TimeoutExpired:
        log(f"Killing self-started headless Godot pid={process.pid}")
        try:
            os.killpg(process.pid, signal.SIGKILL)
        except OSError:
            process.kill()
        process.wait(timeout=3)


def print_status(project: Path) -> int:
    entry = entry_for(project)
    live = entry_is_live_for_project(project, entry)
    payload = {
        "worktree_root": str(git_worktree_root(Path.cwd())),
        "godot_project_path": str(project),
        "registry_path": str(registry_path()),
        "registry_entry": entry,
        "live": live,
        "godot_bin": find_godot_binary(),
    }
    print(json.dumps(payload, indent=2, sort_keys=True))
    return 0 if live else 1


def bridge_env(project: Path) -> dict[str, str]:
    env = os.environ.copy()
    env["GODOT_MCP_CONFIG_VERSION"] = "1"
    env["GODOT_MCP_PROJECT_PATH"] = str(project)
    # Intentionally do not set GODOT_MCP_EDITOR_PORT: the toolkit registry solves
    # per-worktree discovery and avoids static port assignment.
    return env


def run_bridge(project: Path, bridge_args: list[str]) -> int:
    args = bridge_args or ["npx", "-y", "@npgamedev/godot-mcp-server"]
    log(f"Starting MCP bridge for {project}: {' '.join(args)}")
    process = subprocess.Popen(args, cwd=str(project), env=bridge_env(project))

    def forward(signum: int, _frame: Any) -> None:
        if process.poll() is None:
            try:
                process.send_signal(signum)
            except OSError:
                pass

    previous_int = signal.signal(signal.SIGINT, forward)
    previous_term = signal.signal(signal.SIGTERM, forward)
    try:
        return process.wait()
    finally:
        signal.signal(signal.SIGINT, previous_int)
        signal.signal(signal.SIGTERM, previous_term)


def main() -> int:
    parser = argparse.ArgumentParser(description="Worktree-aware Godot MCP launcher")
    parser.add_argument("--status", action="store_true", help="Print worktree/project/registry status and exit")
    parser.add_argument("--project-path", action="store_true", help="Print computed Godot project path and exit")
    parser.add_argument("--no-start", action="store_true", help="Do not auto-start Godot; fail if no live editor exists")
    parser.add_argument("--", dest="separator", action="store_true", help=argparse.SUPPRESS)
    parser.add_argument("bridge_args", nargs=argparse.REMAINDER, help="Optional bridge command override")
    ns = parser.parse_args()

    worktree = git_worktree_root(Path.cwd())
    project = godot_project_root(worktree)

    if ns.project_path:
        print(project)
        return 0
    if ns.status:
        return print_status(project)

    started: subprocess.Popen[Any] | None = None
    entry = entry_for(project)
    if entry_is_live_for_project(project, entry):
        log(f"Reusing live Godot editor pid={int(entry['pid'])} port={int(entry['port'])} project={project}")
    else:
        if ns.no_start:
            raise SystemExit(f"No live Godot editor registered for {project}")
        started = start_headless_editor(project)
        entry = wait_for_registry(project, started)
        log(f"Headless Godot registered pid={int(entry['pid'])} port={int(entry['port'])}")

    try:
        return run_bridge(project, ns.bridge_args)
    finally:
        terminate_started(started)


if __name__ == "__main__":
    raise SystemExit(main())
