core_types_lib = static_library(
    'core_types_lib',
    sources: [
        'std_base_types.cpp',
        'control_object.cpp',
        'error_code.cpp',
        'scope_type.cpp',
        'variable_type.cpp',
        hs_proto_generated_headers,
    ],
    dependencies: common_deps,
    link_with: [utils_lib, hs_proto_lib],
    include_directories: root_include,
)

core_types_lib_dep = declare_dependency(
    link_with: core_types_lib,
    include_directories: root_include,
    dependencies: common_deps,
)
