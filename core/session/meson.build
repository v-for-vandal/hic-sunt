core_session_lib = static_library(
    'core_session_lib',
    sources: [
        'effect_instance.cpp',
        'lua_scope_change_set.cpp',
        hs_proto_generated_headers,
    ],
    dependencies: [common_deps, sol2_dep, lua_dep],
    link_with: [utils_lib, core_ruleset_lib],
    include_directories: root_include,
)

core_session_tests = executable(
    'unit_tests',
    sources: [
        'test_main.cpp',
        'effect_instance_test.cpp',
        hs_proto_generated_headers,
    ],
    dependencies: [common_deps, gtest_dep, sol2_dep, lua_dep],
    link_with: [core_session_lib, core_ruleset_lib, hs_proto_lib],
    include_directories: root_include,
)

test('core.session', core_session_tests, env: environment(test_env))
