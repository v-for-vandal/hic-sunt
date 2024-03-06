core_region_lib = static_library('core_region_lib',
  sources: [
  'cell.cpp',
  'region.cpp',
  'pnl_statement.cpp',
  hs_proto_generated_headers,
  ],
  dependencies : common_deps,
  link_with: [utils_lib, core_utils_lib, core_ruleset_lib],
  include_directories: root_include)

core_region_tests = executable('unit_tests',
  sources: [
    'test_main.cpp',
    'region_test.cpp',
    hs_proto_generated_headers
    ],
  dependencies : common_deps + gtest_dep,
  link_with: [core_geometry_lib, core_region_lib, hs_proto_lib],
  include_directories: root_include,
  )

test('core.region', core_region_tests, env: environment(test_env))


