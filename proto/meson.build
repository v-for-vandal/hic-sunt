protobuf_dep = dependency('protobuf', version: '>=25')

if not protobuf_dep.found()
  error('Can not find protobuf')
endif

protoc_prg = find_program('protoc')

proto_gen = generator(protoc_prg,
  output    : ['@BASENAME@.pb.cc', '@BASENAME@.pb.h'],
  arguments : ['--proto_path=@CURRENT_SOURCE_DIR@', '--cpp_out=@BUILD_DIR@', '@INPUT@'])

proto_generated = proto_gen.process(
  'city/placement_rules.proto',
  'city/improvements.proto',
  preserve_path_from : meson.current_source_dir())

message('protobuf dep is found: ', protobuf_dep.found())
message('protobuf dep name is: ', protobuf_dep.name())
message('protoc full path: ', protoc_prg.full_path())

hs_proto_lib = static_library('hs_proto_lib',
  proto_generated,
  dependencies: [protobuf_dep]
  )
