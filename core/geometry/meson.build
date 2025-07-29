core_geometry_lib = static_library('core_geometry_lib',
  sources: [
    'coord_system.cpp',
    'surface_shape.cpp',
    hs_proto_generated_headers,
  ],
  dependencies : common_deps,
  link_with: [utils_lib, hs_proto_lib],
  include_directories: root_include)

core_geometry_tests = executable('unit_tests',
  sources: [
    'coords_test.cpp',
    'surface_test.cpp',
    'test_main.cpp',
    hs_proto_generated_headers,
  ],
  dependencies : common_deps + gtest_dep,
  link_with: [core_geometry_lib],
  include_directories: root_include,
  )

test('core.geometry', core_geometry_tests, env: environment(test_env))

