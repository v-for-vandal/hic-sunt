_core_scope_lib_dependencies = [
    # common deps
    common_deps,

    # proto and utils
    utils_lib_dep,
    hs_proto_lib_dep,
]

core_scope_lib = static_library(
    'core_scope_lib',
    sources: [
        'scope.cpp',
        'scope_change_set.cpp',
        'scoped_object.cpp',
        'numeric_variable.cpp',
        'string_variable.cpp',
    ],
    dependencies: _core_scope_lib_dependencies,
)

core_scope_lib_dep = declare_dependency(
    link_with: core_scope_lib,
    dependencies: _core_scope_lib_dependencies,
)

core_scope_ut_lib = static_library(
    'core_scope_ut_lib',
    sources: [
        'scope_ut.cpp',
    ],
    dependencies: [common_deps, gtest_dep, core_scope_lib_dep, core_ruleset_ut_lib_dep],
)

core_scope_ut_lib_dep = declare_dependency(
    link_with: core_scope_ut_lib,
    dependencies: [
        common_deps,
        gtest_dep,
        lua_dep,
        core_scope_lib_dep,
        core_ruleset_ut_lib_dep,
    ],
)

core_scope_tests = executable(
    'unit_tests',
    sources: [
        'test_main.cpp',
        'scope_test.cpp',
        'scope_change_set_test.cpp',
    ],
    dependencies: [core_scope_ut_lib_dep],
)

test('core.scope', core_scope_tests, env: environment(test_env))
