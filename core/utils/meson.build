core_utils_lib = static_library('core_utils_lib',
  sources: [
  'comb.cpp',
  hs_proto_generated_headers,
  ],
  dependencies : common_deps,
  link_with: [utils_lib],
  include_directories: root_include)

core_utils_tests = executable('unit_tests',
  sources: [
    'test_main.cpp',
    'minmax_test.cpp',
    'non_null_ptr_test.cpp',
    ],
  dependencies : common_deps + gtest_dep,
  link_with: [core_utils_lib],
  include_directories: root_include,
  )

test('core.utils', core_utils_tests, env: environment(test_env))

