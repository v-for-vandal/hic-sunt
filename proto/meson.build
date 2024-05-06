protobuf_dep = dependency('protobuf', version: '>=25')

if not protobuf_dep.found()
  error('Can not find protobuf')
endif

protoc_prg = find_program('protoc')

proto_gen = generator(protoc_prg,
  output    : ['@BASENAME@.pb.cc', '@BASENAME@.pb.h'],
  arguments : ['--proto_path=@CURRENT_SOURCE_DIR@', '--cpp_out=@BUILD_DIR@', '@INPUT@'])

proto_gen_hh = generator(protoc_prg,
  output    : ['@BASENAME@.pb.h'],
  arguments : ['--proto_path=@CURRENT_SOURCE_DIR@', '--cpp_out=@BUILD_DIR@', '@INPUT@'])

hs_proto_schemas = [
  'region/cell.proto',
  'region/improvement.proto',
  'region/region.proto',
  'l10n/localized_name.proto',
  'terra/cell.proto',
  'terra/world.proto',
  'ruleset/region_improvements.proto',
  'ruleset/placement_rules.proto',
  'ruleset/biome.proto',
  'ruleset/resources.proto',
  'ruleset/jobs.proto',
  'render/atlas.proto',
  'render/render.proto',
  ]

hs_proto_generated = proto_gen.process(
  hs_proto_schemas,
  preserve_path_from : meson.current_source_dir())

hs_proto_generated_headers = proto_gen_hh.process(
  hs_proto_schemas,
  preserve_path_from : meson.current_source_dir())

#proto_files = [
#  'city/placement_rules.proto',
#  'city/improvements.proto',
#  'l10n/localized_name.proto',
#  ]
#
#proto_gen_cc = custom_target('proto_gen_cc',
#  input: proto_files,
#  output: ['@BASENAME@.pb.cc'],
#  command: [
#    protoc_prg,
#    '--proto_path=@CURRENT_SOURCE_DIR@',
#    '--cpp_out=@BUILD_DIR@'
#    ],
#  )
#proto_gen_hh = custom_target('proto_gen_hh',
#  input: proto_files,
#  output: ['@BASENAME@.pb.h'],
#  command: [
#    protoc_prg,
#    '--proto_path=@CURRENT_SOURCE_DIR@',
#    '--cpp_out=@BUILD_DIR@'
#    ],
#  )

hs_proto_lib = static_library('hs_proto_lib',
  hs_proto_generated,
  dependencies: [protobuf_dep,]
  )

#hs_proto_lib_inc = [
#   meson.current_build_dir() + '/libhs_proto_lib.a.p/'
