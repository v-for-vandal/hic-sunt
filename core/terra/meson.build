core_terra_lib = static_library('core_terra_lib',
  sources: [
  'cell.cpp',
  'world.cpp',
  fbs_generated,
  ],
  dependencies : common_deps,
  link_with: [utils_lib],
  include_directories: root_include)

#core_terra_tests = executable('unit_tests',
#  'coords_test.cpp',
#  dependencies : common_deps + gtest_dep,
#  link_with: [core_geometry_lib, core_terra_lib],
#  include_directories: root_include,
#  )
#
#test('core.terra', core_terra_tests, env: environment(test_env))

