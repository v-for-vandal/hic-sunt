sources = run_command('find', './', '-name', '.godot', '-prune', '-o', '-type', 'f', '!', '-name', '.git*', '-print', check:true).stdout().strip().split('\n')

# message('Detected sources: @0@'.format(sources))

install_dir = 'content/worldgen-basic'

custom_target('worldgen-basic',
  output: 'data.pck',
  input: sources,
  command: [godot, '--path', meson.current_source_dir(), '--headless', '--export-pack', 'Linux', meson.current_build_dir() / 'data.pck'],
  install: true,
  install_dir: install_dir
  )

install_data('module.cfg', install_dir: install_dir)
