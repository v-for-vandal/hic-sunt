core_scope_lib = static_library('core_scope_lib',
  sources: [
  'scope.cpp',
  'scoped_object.cpp',
  'numeric_variable.cpp',
  'string_variable.cpp',
  hs_proto_generated_headers,
  ],
  dependencies : common_deps,
  link_with: [utils_lib, hs_proto_lib],
  include_directories: root_include)


core_scope_tests = executable('unit_tests',
  sources: [
    'test_main.cpp',
    'scope_test.cpp',
    hs_proto_generated_headers
    ],
  dependencies : common_deps + gtest_dep,
  link_with: [core_scope_lib, hs_proto_lib],
  include_directories: root_include,
  )

test('core.scope', core_scope_tests, env: environment(test_env))
