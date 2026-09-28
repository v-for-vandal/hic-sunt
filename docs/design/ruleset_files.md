# Ruleset files

Rulesets are stored in directories and are loaded in the order given in
game settings or via command flags.

Each folder must have a predefined folder structure:
* 'improvements/' - stores improvements
* 'biomes/' - stores biomes
* 'resources' - stores resources
* 'rendering' - currently not used
* 'jobs' - stores jobs
* 'projects' - stores city and empire projects
* 'effects' - stores effects
* 'variables' - stores variables

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

Due to underlying yaml -> json -> protobuf transcoding, there are some quirks,
e.g. you can use string literal "true" in place of bool. Please don't do this,
we will get rid of it as soon as we can.

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

Roots are processed in the given order. All accepted files from the
first root are applied before files from the second root, and so on.

## Override rules

Files are applied in the order described above. Later files override earlier objects with the
same `id` inside the same ruleset collection.

For example, if `projects/01_base.yaml` defines `project.one` and `projects/02_override.txt`
defines `project.one` again, the object from `02_override.txt` replaces the earlier one.

This is an object-level replacement, not a deep merge of individual fields.
