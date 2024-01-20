subdir('geometry')
subdir('material')
subdir('terra')
subdir('timeline')
subdir('substance')

core_lib = static_library('core_lib', 'core.cpp',
  link_with: [
    core_terra_lib,
    core_timeline_lib,
    core_substance_lib,
    core_material_lib,
    ]
  )

