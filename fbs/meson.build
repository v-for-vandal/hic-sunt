flatc_prg = find_program('flatc')

fbs_generated = custom_target(
  'fbs_generated_headers',
  output: ['world_generated.h', 'ruleset_generated.h'],
  input: ['world.fbs', 'rules/ruleset.fbs'],
  command: [flatc_prg, '--cpp', '-o', 'fbs', '@INPUT@']
  )
