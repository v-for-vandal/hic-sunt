sources = run_command('find', './', '-type', 'f', '!', '-name', '*.cache', check:true).stdout().strip().split('\n')

message('Current build dir: @0@'.format(meson.current_build_dir()))

custom_target('worldgen-basic',
  output: 'worldgen-basic.pck',
  input: sources,
  command: [godot, '--path', meson.current_source_dir(), '--headless', '--export-pack', 'Linux', meson.current_build_dir() / 'worldgen-basic.pck'],
  install: true,
  install_dir: 'content'
  )
