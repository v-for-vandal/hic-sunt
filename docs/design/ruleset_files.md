# Ruleset files

Ruleset data is loaded from one or more ruleset root directories passed to `RuleSet::Load`.
Each root may contain the known ruleset subdirectories, such as `improvements`, `biomes`,
`resources`, `jobs`, `rendering`, `projects`, `variables`, and `effects`.

## Supported formats

Ruleset files may use either format:

- `.txt` — protobuf text format
- `.yaml` / `.yml` — YAML converted to protobuf JSON mapping and then parsed as protobuf

In YAML files, protobuf `map` fields are written as YAML dictionaries:

```yaml
jobs:
  core.job.woodcutter: 1
```

not as textproto-style `{ key, value }` entries.

YAML anchors, aliases, and custom tags are intentionally not supported.

YAML scalars are converted to JSON before protobuf parsing. Quoted YAML scalars become JSON
strings, while plain `true`, `false`, `null`, and numeric-looking scalars become the matching
JSON scalar types. The final parse is performed by protobuf C++ JSON transcoding. That parser
currently enables legacy syntax internally, so it may accept some non-canonical scalar forms,
such as quoted booleans for protobuf `bool` fields.

## File order

For each ruleset root and subdirectory:

1. Files are collected recursively.
2. Only `.txt`, `.yaml`, and `.yml` files are considered.
3. Files in the same physical directory may not have the same base name with different
   ruleset extensions. For example, `jobs/base.txt` and `jobs/base.yaml` in the same
   directory are a conflict. All files in such a conflicting group are ignored.
4. Remaining files are sorted lexicographically by their path relative to the ruleset
   subdirectory, with the final extension removed.

Example order inside `projects`:

```text
01_base.yaml
02_more.txt
nested/01_nested.yaml
```

Roots are processed in the order provided to `RuleSet::Load`. All accepted files from the
first root are applied before files from the second root, and so on.

## Override rules

Files are applied in the order described above. Later files override earlier objects with the
same `id` inside the same ruleset collection.

For example, if `projects/01_base.yaml` defines `project.one` and `projects/02_override.txt`
defines `project.one` again, the object from `02_override.txt` replaces the earlier one.

This is an object-level replacement, not a deep merge of individual fields.
