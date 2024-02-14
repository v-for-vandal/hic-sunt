godot_module_sources = [
    'register_types.cpp',
    'system/system.cpp',
    'game/ruleset_object.cpp',
    'terra/world_object.cpp',
    'region/region_object.cpp',
]


hic_sunt_godot = shared_library('hic_sunt_godot',
  sources: godot_module_sources + hs_proto_generated_headers,
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
  install_dir: meson.source_root() + '/ui/godot/project/module')


