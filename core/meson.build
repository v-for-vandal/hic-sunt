subdir('utils')
subdir('geometry')
subdir('types')
subdir('ruleset')
subdir('scope')
subdir('region')
subdir('terra')
subdir('session')
subdir('system')

_core_lib_dependencies = [
    # all other top-level modules
    hs_proto_lib_dep,
    core_session_lib_dep,
    core_terra_lib_dep,
    core_region_lib_dep,
]

core_lib = static_library(
    'core_lib',
    'core.cpp',
    fbs_generated,
    dependencies: _core_lib_dependencies,
    #link_with: [
    #  hs_proto_lib,
    #  core_utils_lib,
    #  core_terra_lib,
    #  core_region_lib,
    #  core_scope_lib,
    #  core_system_lib,
    #  core_session_lib,
    #  core_types_lib,
    #  ]
)

core_lib_dep = declare_dependency(
    link_with: core_lib,
    dependencies: _core_lib_dependencies,
)
