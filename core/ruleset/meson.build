core_ruleset_lib = static_library('core_ruleset_lib',
  sources: [
  'effect.cpp',
  'ruleset.cpp',
  'ruleset_base.cpp',
  fbs_generated,
  hs_proto_generated_headers,
  ],
  dependencies : common_deps,
  link_with: [utils_lib, core_types_lib],
  include_directories: root_include)

core_ruleset_tests = executable('unit_tests',
  sources: [
    'test_main.cpp',
    'effect_test.cpp',
    'ruleset_load_test.cpp',
    hs_proto_generated_headers,
    ],
  dependencies : common_deps + gtest_dep,
  link_with: [core_ruleset_lib, hs_proto_lib],
  include_directories: root_include,
  )

test('core.ruleset', core_ruleset_tests, env: environment(test_env))
