godot = find_program('godot')

godot_project_dir = meson.current_source_dir() / 'project'

test(
    'godot-gut',
    godot,
    args: [
        '--headless',
        '-s',
        'addons/gut/gut_cmdln.gd',
        '-gexit',
    ],
    workdir: godot_project_dir,
    env: {
        'ASAN_OPTIONS': 'alloc_dealloc_mismatch=0:detect_leaks=0:halt_on_error=0:abort_on_error=0',
        'UBSAN_OPTIONS': 'halt_on_error=0:abort_on_error=0:print_summary=1:print_stacktrace=1',
        'MSAN_OPTIONS': 'halt_on_error=0:abort_on_error=0:print_summary=1:print_stacktrace=1',
        'MALLOC_PERTURB_': '0',
    },
    timeout: 120,
)

subdir('module')

# This mod organization structure doesn't really works :(
#subdir('content')
