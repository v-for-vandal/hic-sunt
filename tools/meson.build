ruleset_validator = executable(
    'ruleset_validator',
    sources: [
        'ruleset_validator.cpp',
    ],
    dependencies: [
        core_ruleset_lib_dep,
    ],
)

godot_gamedata_dir = meson.source_root() / 'ui/godot/project/gamedata'

test(
    'ruleset.gamedata.core_v1_0',
    ruleset_validator,
    args: [
        godot_gamedata_dir / 'core',
        godot_gamedata_dir / 'v1.0',
    ],
    env: environment(test_env),
)

test(
    'ruleset.gamedata.core_utest',
    ruleset_validator,
    args: [
        godot_gamedata_dir / 'core',
        godot_gamedata_dir / 'utest',
    ],
    env: environment(test_env),
)
