hic_sunt_godot = shared_library('hic_sunt_godot',
  'init.cpp',
  'terra/world_object.cpp',
  #'terra/cell_object.cpp',
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


