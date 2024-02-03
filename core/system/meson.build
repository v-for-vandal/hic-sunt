core_system_lib = static_library('core_system_lib',
  sources: [
  'system.cpp',
  fbs_generated,
  hs_proto_generated_headers,
  ],
  dependencies : common_deps,
  link_with: [utils_lib, core_terra_lib, core_ruleset_lib],
  include_directories: root_include)

#core_system_tests = executable('unit_tests',
#  'test_main.cpp',
#  'world_test.cpp',
#  dependencies : common_deps + gtest_dep,
#  link_with: [core_geometry_lib, core_system_lib],
#  include_directories: root_include,
#  )
#
#test('core.system', core_system_tests, env: environment(test_env))


