# AGENTS.md for `core`

## Scope and purpose

This directory contains the main domain model and foundational game logic. It is built as a set of small static libraries that are linked into `core_lib`.

The `core` subtree is organized into the following modules:

- `utils/` — low-level helpers and generic support code
- `geometry/` — hex-grid coordinate types, boxes, surface shapes, and surface storage/views
- `types/` — shared base types and control objects
- `ruleset/` — loading and querying ruleset data from protobuf text files
- `region/` — regions, region cells, improvements, aggregation, and region-level serialization
- `scope/` — variable scopes, inheritance, and modifier evaluation
- `terra/` — world and plane model built from regions
- `system/` — currently minimal / mostly disabled integration layer

When editing files under `core/`, prefer changes that preserve module boundaries and keep domain concepts explicit.

## Architectural guidance

The code in `core/` is heavily type-oriented and template-based. Many public types are parameterized by `BaseTypes`, with `StdBaseTypes` as the default implementation.

Important cross-cutting patterns:

- Many entities inherit from `scope::ScopedObject<BaseTypes>`.
- `Scope` is the preferred mechanism for extensible numeric and string state.
- Serialization is implemented with `SerializeTo(...)` and `ParseFrom(...)` free functions.
- Some modules maintain both persistent state and derived / ephemeral cached state.
- Several headers include matching `.inl` files for template implementations.

Before changing behavior, identify whether the affected data is:
- persistent and serialized,
- derived and recomputed,
- local to one module,
- or shared across multiple modules.

## Module notes

### `geometry/`
This module defines strongly typed axial/cube-coordinate helpers for hex grids.

Key ideas:
- Use the typed axis and delta wrappers instead of raw integers where possible.
- `Surface` and `SurfaceView` represent storage and traversal over shaped map areas.
- Shape semantics matter: bounding allocation may be larger than the actually valid cells.
- Use `Contains(...)` before assuming coordinates are valid for a shape.

Be careful:
- Coordinate and shape code is foundational; avoid casual API changes.
- Equality and iteration behavior can affect multiple higher-level modules.
- This area has unit tests; preserve or extend them when changing logic.

### `scope/`
This module implements scoped variables with inheritance and modifiers.

Key ideas:
- `Scope` stores numeric and string variables separately.
- Parent scopes are part of the resolution chain.
- Modifier explanation APIs are intended for diagnostics/UI-facing introspection.
- Copying is intentionally disabled for `Scope`; preserve identity expectations.

Be careful:
- Scope behavior propagates into `region/`, `terra/`, and anything using `ScopedObject`.
- Prefer fixing root behavior in `Scope` rather than patching callers.

### `region/`
This module models a region and its cells.

Key ideas:
- `Region` owns a `Surface<Cell<...>>`.
- Region-level setters maintain aggregated or cached state.
- Do not bypass region-level mutation paths when the region tracks derived data.
- `Cell` also carries scope-derived state through `ScopedObject`.

Be careful:
- Prefer `Region::SetBiome`, `SetFeature`, and related region-level APIs over direct cell mutation.
- Directly changing `Cell` internals can desynchronize aggregates like biome counts or improvement indexes.
- Region serialization and equality are tested and should remain stable.

### `terra/`
This module models larger world structures.

Key ideas:
- `World` contains planes.
- `Plane` contains regions and surface layout metadata.
- Region identity lookup is maintained in indexes in addition to surface placement.
- Distance methods are world-generation oriented, not general movement/pathfinding APIs.

Be careful:
- Keep plane surface storage and region index structures consistent.
- Preserve scope relationships between world, plane, region, and cell objects.

### `ruleset/`
This module loads protobuf text rules from the filesystem and builds lookup indexes.

Key ideas:
- `RuleSet` builds typed lookup tables on top of `RuleSetBase`.
- Loading currently expects multiple textproto inputs from a directory.
- Failures should report useful errors through the existing error collection path.

Be careful:
- Current loading behavior is not a full merge system.
- Avoid assuming missing files are acceptable unless you intentionally implement that behavior.

### `types/` and `utils/`
These modules provide shared primitives and helper abstractions.

Be careful:
- These files are widely reused; seemingly small changes can fan out quickly.
- Pointer wrappers have semantic intent beyond plain smart pointers. Preserve those semantics.

## Editing rules

When making changes in `core/`, follow these rules:

1. Prefer small, local, behavior-preserving edits unless the task clearly requires architectural change.
2. Preserve template and free-function serialization patterns already used in the module.
3. Do not replace strongly typed coordinate wrappers with primitive integers.
4. Do not bypass aggregate-maintaining APIs in `region/` or index-maintaining APIs in `terra/`.
5. Keep scope parent/child relationships valid after construction, parsing, or mutation.
6. Preserve deleted copy operations where identity or ownership semantics depend on them.
7. When editing headers with matching `.inl` files, keep declarations and definitions aligned.
8. Prefer extending existing tests over introducing ad hoc debug-only behavior.

## Testing guidance

There are module-local unit tests for at least:

- `core.utils`
- `core.geometry`
- `core.region`
- `core.scope`
- `core.terra`

`core.system` tests are currently commented out / disabled in the build.

If you change behavior in one module, check whether nearby modules rely on it transitively. In particular:

- `geometry` changes can affect `region` and `terra`
- `scope` changes can affect `region`, `terra`, and serialization expectations
- `ruleset` changes can affect region PnL or lookup behavior
- `utils` and `types` changes can affect almost everything

## Known pitfalls and maintenance notes

- Some code paths still contain TODOs or transitional comments from older designs.
- Some legacy per-cell/per-region data APIs appear to have been superseded by `Scope`.
- Derived caches and indexes must stay synchronized with source state.
- Not every subsystem has equally complete test coverage.
- `system/` is not the authoritative place to add new core gameplay logic right now.

## Preferred agent behavior in this directory

When working in `core/`, an agent should:

- read the affected module's public headers first,
- identify related `.inl`, `.cpp`, serialization, and test files,
- preserve invariants before optimizing or refactoring,
- add or update tests when changing observable behavior,
- document assumptions in code comments only when they clarify a real invariant.

If uncertain, prefer conservative changes that respect the current data model and module boundaries.