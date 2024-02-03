core_terra_lib = static_library('core_terra_lib',
  sources: [
  'cell.cpp',
  'world.cpp',
  fbs_generated,
  hs_proto_generated_headers,
  ],
  dependencies : common_deps,
  link_with: [utils_lib],
  include_directories: root_include)

core_terra_tests = executable('unit_tests',
  sources: [
    'test_main.cpp',
    'world_test.cpp',
    hs_proto_generated_headers
    ],
  dependencies : common_deps + gtest_dep,
  link_with: [core_geometry_lib, core_terra_lib, hs_proto_lib],
  include_directories: root_include,
  )

test('core.terra', core_terra_tests, env: environment(test_env))

