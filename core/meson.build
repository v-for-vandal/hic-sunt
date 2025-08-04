subdir('utils')
subdir('geometry')
subdir('ruleset')
subdir('region')
subdir('terra')
subdir('system')
subdir('types')

core_lib = static_library('core_lib',
  'core.cpp',
  fbs_generated,
  link_with: [
    hs_proto_lib,
    core_utils_lib,
    core_terra_lib,
    core_region_lib,
    core_system_lib,
    core_types_lib,
    ]
  )

