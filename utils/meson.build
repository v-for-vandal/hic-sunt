utils_lib = static_library(
    'utils_lib',
    dependencies: common_deps,
)

utils_lib_dep = declare_dependency(
    link_with: utils_lib,
    dependencies: common_deps,
)

test_main_config = configuration_data()
test_main_config.set_quoted('DEFAULT_TEST_RESOURCES_DIR', test_resources_dir)

test_main_cpp = configure_file(
    input: 'test_main.cpp.in',
    output: 'test_main.cpp',
    configuration: test_main_config,
)

test_utils_lib = static_library(
    'test_utils_lib',
    sources: [
        'test_data.cpp',
    ],
    dependencies: common_deps + gtest_dep,
)

test_utils_lib_dep = declare_dependency(
    link_with: test_utils_lib,
    dependencies: common_deps + gtest_dep,
)

utils_tests = executable(
    'unit_tests',
    test_main_cpp,
    dependencies: [utils_lib_dep, test_utils_lib_dep],
)

test('utils', utils_tests, env: environment(test_env))
