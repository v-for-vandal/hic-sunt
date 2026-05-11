_core_session_lib_dependencies = [
    # common deps
    common_deps,
    utils_lib_dep,

    # lua
    sol2_dep,
    lua_dep,

    # other core libs
    core_ruleset_lib_dep,
]

core_session_lib = static_library(
    'core_session_lib',
    sources: [
        'effect_executor.cpp',
        'effect_instance.cpp',
        'lua_scope_change_set.cpp',
        'session.cpp',
    ],
    dependencies: _core_session_lib_dependencies,
)

core_session_lib_dep = declare_dependency(
    link_with: core_session_lib,
    dependencies: _core_session_lib_dependencies,
)

core_session_ut_lib = static_library(
    'core_session_ut_lib',
    sources: [
        'session_ut.cpp',
    ],
    dependencies: [
        common_deps,
        gtest_dep,
        core_scope_lib_dep,
        core_ruleset_ut_lib_dep,
        core_scope_ut_lib_dep,
    ],
)

core_session_ut_lib_dep = declare_dependency(
    link_with: core_session_ut_lib,
    dependencies: [
        common_deps,
        gtest_dep,
        core_scope_lib_dep,
        core_ruleset_ut_lib_dep,
        core_scope_ut_lib_dep,
        hs_proto_lib_dep,
    ],

)

core_session_tests = executable(
    'unit_tests',
    sources: [
        'test_main.cpp',
        'effect_executor_test.cpp',
        'effect_instance_test.cpp',
        'session_test.cpp',
    ],
    dependencies: [core_session_ut_lib_dep, sol2_dep, lua_dep],
)

test('core.session', core_session_tests, env: environment(test_env))
