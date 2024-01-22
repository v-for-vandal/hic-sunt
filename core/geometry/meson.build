core_geometry_lib = static_library('core_geometry_lib',
  'coord_system.cpp',
  dependencies : common_deps,
  link_with: [utils_lib],
  include_directories: root_include)

core_geometry_tests = executable('unit_tests',
  'coords_test.cpp',
  'surface_test.cpp',
  'test_main.cpp',
  dependencies : common_deps + gtest_dep,
  include_directories: root_include,
  )

test('core.geometry', core_geometry_tests, env: environment(test_env))

