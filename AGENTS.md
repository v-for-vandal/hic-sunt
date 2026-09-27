# AGENTS.md

## Build

Use the Clang native file from the repository. Do not configure the build directory with the default compiler.

```sh
meson setup --native-file clang.native.ini --prefix /absolute/path/to/ui/godot/project build
meson compile -C build
```

If `build` is already configured correctly, only run:

```sh
meson compile -C build
```

## Tests

```sh
meson test -C build --print-errorlogs
```

## Formatting

Run formatting through the Meson/Ninja target:

```sh
ninja -C build clang-format
```

Do not run ad-hoc `clang-format` commands manually; use the target above so formatting matches the project setup.
