flatc_prg = find_program('flatc')

fbs_generated = custom_target(
  'world_generated.h',
  output: ['world_generated.h'],
  input: 'world.fbs',
  command: [flatc_prg, '--cpp', '-o', 'fbs', '@INPUT@']
  )
