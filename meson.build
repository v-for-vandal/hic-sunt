project('hic-sunt', 'cpp', version: '0.0.1',
  default_options : [
    'cpp_std=c++20',
    'warning_level=3',
    'prefix=',
    'wrap_mode=forcefallback',

    ])
# TODO: set warning_level=everything when next version of meson is released
#
add_project_arguments([
  '-Wno-gcc-compat',
  ], language: 'cpp', )

fmt_dep = dependency('fmt', version: '>=9.1')

entt_dep = dependency('entt')

spdlog_dep = dependency('spdlog')

flatbuffer_dep = dependency('flatbuffers', static: true)


gtest_dep = dependency('gtest')

# TODO: Fix - our build type is neither debug nor release :(
#if get_option('buildtype') == 'release'
#  add_project_arguments('-DSPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_WARN', language:'cpp')
#else
#  add_project_arguments('-DSPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_DEBUG', language:'cpp')
#endif


subdir('fbs')
subdir('proto')

common_deps = [fmt_dep, entt_dep, spdlog_dep, flatbuffer_dep, protobuf_dep]

root_include = include_directories([
    '.',
    'fbs',
    'submodules/mdspan/include/experimental',
    'submodules/godot-4.2',
    ])

test_resources_dir = meson.source_root() / 'test-resources'
test_env = {
  'TEST_RESOURCES_DIR' :  test_resources_dir,
  'ASAN_OPTIONS' : 'alloc_dealloc_mismatch=0'
  }

subdir('utils')
subdir('core')

godot_dep = dependency('godot-cpp')
#godot_subproj=cmake.subproject('godot-cpp')
#godot_dep = godot_subproj.dependency('godot-cpp')

subdir('ui/godot/')
