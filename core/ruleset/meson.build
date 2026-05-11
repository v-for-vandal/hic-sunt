_core_ruleset_lib_dependencies = [
    # common deps
    common_deps,
    # lua related things
    sol2_dep,
    lua_dep,
    # our libs
    core_types_lib_dep,
    hs_proto_lib_dep,
]

core_ruleset_lib = static_library(
    'core_ruleset_lib',
    sources: [
        'effect.cpp',
        'ruleset.cpp',
        'ruleset_base.cpp',
        fbs_generated,
    ],
    dependencies: _core_ruleset_lib_dependencies,
    link_with: [
        utils_lib,
    ],
)

core_ruleset_lib_dep = declare_dependency(
    include_directories: root_include,
    link_with: core_ruleset_lib,
    dependencies: _core_ruleset_lib_dependencies,

)

core_ruleset_ut_lib = static_library(
    'core_ruleset_ut_lib',
    sources: [
        'ruleset_ut.cpp',
    ],
    dependencies: [core_ruleset_lib_dep, gtest_dep],
)

core_ruleset_ut_lib_dep = declare_dependency(
    include_directories: root_include,
    link_with: core_ruleset_ut_lib,
    dependencies: [core_ruleset_lib_dep, gtest_dep],
)

core_ruleset_tests = executable(
    'unit_tests',
    sources: [
        'test_main.cpp',
        'effect_test.cpp',
        'ruleset_load_test.cpp',
        'variable_definition_test.cpp',
    ],
    dependencies: [
        core_ruleset_ut_lib_dep,
    ],
)

test('core.ruleset', core_ruleset_tests, env: environment(test_env))
