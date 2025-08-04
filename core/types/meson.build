core_types_lib = static_library('core_types_lib',
  sources: [
  'std_base_types.cpp',
  'control_object.cpp',
  hs_proto_generated_headers,
  ],
  dependencies : common_deps,
  link_with: [utils_lib, hs_proto_lib],
  include_directories: root_include)

