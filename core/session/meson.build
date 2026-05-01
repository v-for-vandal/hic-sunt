core_session_lib = static_library(
    'core_session_lib',
    sources: [
        'effect_instance.cpp',
        hs_proto_generated_headers,
    ],
    dependencies: [common_deps, sol2_dep, lua_dep],
    link_with: [utils_lib, core_ruleset_lib],
    include_directories: root_include,
)
