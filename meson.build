project('c++  fortress', 'cpp', version: '0.0.1',
  default_options : ['cpp_std=c++20', 'warning_level=3'])
# TODO: set warning_level=everything when next version of meson is released

boost_dep = dependency('boost')

fmt_dep = dependency('fmt', version: '>=9.1')

entt_dep = dependency('entt')

spdlog_dep = dependency('spdlog')

common_deps = [boost_dep, fmt_dep, entt_dep, spdlog_dep]

gtest_dep = dependency('gtest')

godot_cpp_dep = dependency('godot-cpp')

#absl_dep = dependency('abseil-cpp', modules: ['absl::log'])
#absl_dep = dependency('absl', modules: ['log']) # not in ubuntu yet

root_include = include_directories(['.', 'submodules/mdspan/include/experimental'])

test_resources_dir = meson.source_root() / 'test-resources'
test_env = {'TEST_RESOURCES_DIR' :  test_resources_dir}

subdir('utils')
subdir('core')
subdir('ui/godot/')
