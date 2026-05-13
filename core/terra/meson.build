_core_terra_lib_dependencies = [
    # common deps
    common_deps,

    # protobuf
    hs_proto_lib_dep,

    # our other libs
    core_geometry_lib_dep,
    core_region_lib_dep,
]
core_terra_lib = static_library(
    'core_terra_lib',
    sources: [
        'cell.cpp',
        'world.cpp',
        'plane.cpp',
        fbs_generated,
    ],
    dependencies: _core_terra_lib_dependencies,
    include_directories: root_include,
)

core_terra_lib_dep = declare_dependency(
    link_with: core_terra_lib,
    dependencies: _core_terra_lib_dependencies,
)

core_terra_tests = executable(
    'unit_tests',
    sources: [
        'test_main.cpp',
        'plane_test.cpp',
        'world_test.cpp',
    ],
    dependencies: [
        gtest_dep,
        core_terra_lib_dep,
    ],
)

test('core.terra', core_terra_tests, env: environment(test_env))
