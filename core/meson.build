subdir('geometry')
subdir('terra')
subdir('system')

core_lib = static_library('core_lib',
  'core.cpp',
  fbs_generated,
  link_with: [
    core_terra_lib,
    core_system_lib
    ]
  )

