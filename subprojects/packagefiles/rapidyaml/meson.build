project(
  'rapidyaml',
  'cpp',
  license: 'MIT',
  meson_version: '>=0.56.0',
  version: '0.16.0',
  default_options: ['cpp_std=c++11'],
)

rapidyaml_inc = include_directories(
  'src',
  'ext/c4core.src',
)

rapidyaml_lib = static_library(
  'rapidyaml',
  sources: [
    'src/c4/yml/scalar_style.cpp',
    'src/c4/yml/common.cpp',
    'src/c4/yml/version.cpp',
    'src/c4/yml/node_type.cpp',
    'src/c4/yml/reference_resolver.cpp',
    'src/c4/yml/tree.cpp',
    'src/c4/yml/parse.cpp',
    'src/c4/yml/emit_buf.cpp',
    'src/c4/yml/tag.cpp',
    'src/c4/yml/emit_file.cpp',
    'ext/c4core.src/c4/base64.cpp',
    'ext/c4core.src/c4/error.cpp',
    'ext/c4core.src/c4/format.cpp',
    'ext/c4core.src/c4/language.cpp',
    'ext/c4core.src/c4/memory_util.cpp',
    'ext/c4core.src/c4/utf.cpp',
    'ext/c4core.src/c4/version.cpp',
  ],
  include_directories: rapidyaml_inc,
)

rapidyaml_dep = declare_dependency(
  link_with: rapidyaml_lib,
  include_directories: rapidyaml_inc,
)
