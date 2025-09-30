core_scope_lib = static_library('core_scope_lib',
  sources: [
  'scope.cpp',
  'scoped_object.cpp',
  hs_proto_generated_headers,
  ],
  dependencies : common_deps,
  link_with: [utils_lib, hs_proto_lib],
  include_directories: root_include)


