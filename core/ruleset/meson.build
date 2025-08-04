core_ruleset_lib = static_library('core_ruleset_lib',
  sources: [
  'ruleset.cpp',
  'ruleset_base.cpp',
  fbs_generated,
  hs_proto_generated_headers,
  ],
  dependencies : common_deps,
  link_with: [utils_lib],
  include_directories: root_include)

