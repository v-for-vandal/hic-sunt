_core_region_lib_dependencies = [
    # common deps
    common_deps,

    # utils
    utils_lib_dep,
    core_utils_lib_dep,

    # other core libs
    core_ruleset_lib_dep,
]

core_region_lib = static_library(
    'core_region_lib',
    sources: [
        'cell.cpp',
        'region.cpp',
        'pnl_statement.cpp',
    ],
    dependencies: _core_region_lib_dependencies,
)

core_region_lib_dep = declare_dependency(
    link_with: core_region_lib,
    dependencies: _core_region_lib_dependencies,
)

core_region_tests = executable(
    'unit_tests',
    sources: [
        'test_main.cpp',
        'region_test.cpp',
    ],
    dependencies: [
        common_deps,
        gtest_dep,
        core_region_lib_dep,
        core_geometry_lib_dep,
        hs_proto_lib_dep,
        core_scope_ut_lib_dep,
    ],
)

test('core.region', core_region_tests, env: environment(test_env))
