utils_lib = static_library(
    'utils_lib',
    dependencies: common_deps,
)

utils_lib_dep = declare_dependency(
    link_with: utils_lib,
    dependencies: common_deps,
)

utils_tests = executable(
    'unit_tests',
    'test_main.cpp',
    dependencies: [common_deps, gtest_dep, utils_lib_dep],
)

test('utils', utils_tests, env: environment(test_env))
