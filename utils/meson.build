utils_lib = static_library('utils_lib',
  'stb_image.cpp',
  'image.cpp',
  dependencies : common_deps,
  include_directories: root_include,
  )

utils_tests = executable('unit_tests',
  'image_test.cpp',
  'test_main.cpp',
  dependencies: common_deps + gtest_dep,
  link_with: [utils_lib],
  include_directories: root_include,
  )

test('utils', utils_tests, env: environment(test_env))
