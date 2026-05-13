_core_geometry_lib_dependencies = [common_deps, hs_proto_lib_dep, utils_lib_dep]

core_geometry_lib = static_library(
    'core_geometry_lib',
    sources: [
        'coord_system.cpp',
        'surface_shape.cpp',
    ],
    dependencies: _core_geometry_lib_dependencies,
)

core_geometry_lib_dep = declare_dependency(
    link_with: core_geometry_lib,
    dependencies: _core_geometry_lib_dependencies,
)

core_geometry_tests = executable(
    'unit_tests',
    sources: [
        'coords_test.cpp',
        'surface_test.cpp',
        'test_main.cpp',
    ],
    dependencies: [common_deps, gtest_dep, core_geometry_lib_dep],
)

test('core.geometry', core_geometry_tests, env: environment(test_env))
