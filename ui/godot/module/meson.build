godot_module_sources = [
    'register_types.cpp',
    'system/system.cpp',
    'system/surface_event.cpp',
    'game/ruleset_object.cpp',
    'game/pnl_object.cpp',
    'terra/world_object.cpp',
    'terra/plane_object.cpp',
    'region/region_object.cpp',
    hs_proto_generated_headers,
]


hic_sunt_godot = shared_library('hic_sunt_godot',
  sources: godot_module_sources,
  link_args: ['-static-libstdc++'],
  dependencies : [common_deps, godot_dep],
  link_whole: [core_lib],
  cpp_args: [
      '-Wno-unused-parameter',
      '-Wno-extra-semi',
      '-Wno-gnu-anonymous-struct',
      '-Wno-nested-anon-types',

     ],
  include_directories: root_include,
  install: true,
  install_dir: 'module')


godot_module_tests = executable('unit_tests',
  sources: [
    'test_main.cpp',
    'region/region_test.cpp',
    hs_proto_generated_headers,
    ],
  dependencies : [common_deps, gtest_dep, godot_dep],
  link_with: [hic_sunt_godot],
  include_directories: root_include,
  )

test('godot.module', godot_module_tests, env: environment(test_env))
