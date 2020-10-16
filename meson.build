# TODO: cross-compile supports is rudimentary

project('uftrace', 'c', version: '0.9.4')
python_module = import('python')

common_cflags = ['-D_GNU_SOURCE', '-D_DEFAULT_SOURCE', '-D_XOPEN_SOURCE=600']
common_cflags += '-fvisibility=hidden'
common_cflags += '-fno-omit-frame-pointer'
program_shell = find_program('sh', required: true)
program_git = find_program('git', required: true)

cc = meson.get_compiler('c')

if get_option('arch') == 'auto'
    arch = host_machine.cpu_family()
elif get_option('arch') == host_machine.cpu_family()
    arch = host_machine.cpu_family()
else
    error('host compiler `@0@` doesn\'t match host architecture `@1@`'.format(
        host_machine.cpu_family(), get_option('arch')))
endif

################
# dependencies #
################

libcapstone_is_enabled = false
libcapstone_dep = []
if get_option('without-libcapstone').disabled()
    libcapstone_dep = dependency('capstone', required: false)
    if libcapstone_dep.found()
        libcapstone_is_enabled = true
        common_cflags += '-DHAVE_LIBCAPSTONE'
    endif
endif

libdw_is_enabled = false
libdw_dep = []
if get_option('without-libdw').disabled() and get_option('without-libelf').disabled()
    libdw_dep = cc.find_library('dw', required: false)
    if libdw_dep.found()
        libdw_is_enabled = true
        common_cflags += '-DHAVE_LIBDW'
    endif
endif

libelf_is_enabled = false
libelf_dep = []
if get_option('without-libelf').disabled()
    libelf_dep = cc.find_library('elf', required: false)
    if libelf_dep.found()
        libelf_is_enabled = true
        common_cflags += '-DHAVE_LIBELF'
    endif
endif

libluajit_is_enabled = false
libluajit_dep = []
if get_option('without-libluajit').disabled()
    libluajit_dep = dependency('luajit', required: false)
    if libluajit_dep.found()
        libluajit_is_enabled =true
        common_cflags += '-DHAVE_LIBLUAJIT'
    endif
endif

libncurses_is_enabled = false
libncurses_dep = []
if get_option('without-libncurses').disabled()
    libncurses_dep = dependency('ncurses', required: false)
    if libncurses_dep.found()
        libncurses_is_enabled = true
        common_cflags += '-DHAVE_LIBNCURSES'
    endif
endif

libpython3_is_enabled = false
libpython3_dep = []
if get_option('without-libpython').disabled()
    python3_installation = python_module.find_installation('python3', required: false)
    if python3_installation.found()
        libpython3_dep = python3_installation.dependency()
        if libpython3_dep.found()
            libpython3_is_enabled = true
            common_cflags += '-DHAVE_LIBPYTHON3'
        endif
    endif
endif

cxa_demangle_is_enabled = false
cxa_demangle_dep = []
if get_option('without-libstdcxx').disabled()
    have_cxa_demangle_code = run_command('cat', files('check-deps/__cxa_demangle.c')).stdout().strip()
    have_cxa_demangle_result = cc.run(have_cxa_demangle_code, name: 'cxa_demangle check', args: ['-lstdc++'])
    if have_cxa_demangle_result.returncode() == 0
        cxa_demangle_dep = cc.find_library('stdc++', required: true)
        cxa_demangle_is_enabled = true
        common_cflags += '-DHAVE_CXA_DEMANGLE'
        message('Enabling libstdc++')
    else
        message('Disabling libstdc++')
    endif
endif

perf_clockid_is_enabled = false
if get_option('without-perf').disabled()
    have_perf_clockid_code = run_command('cat', files('check-deps/__perf_clockid.c')).stdout().strip()
    have_perf_clockid_result = cc.run(have_perf_clockid_code, name: 'perf clockid check')
    if have_perf_clockid_result.returncode() == 0
        perf_clockid_is_enabled = true
        common_cflags += '-DHAVE_PERF_CLOCKID'
        message('Enabling perf event support')
    else
        message('Disabling perf event support')
    endif
endif

perf_ctxsw_is_enabled = false
if get_option('without-schedule').disabled()
    have_perf_ctxsw_code = run_command('cat', files('check-deps/__perf_context_switch.c')).stdout().strip()
    have_perf_ctxsw_result = cc.run(have_perf_ctxsw_code, name: 'perf context switch check')
    if have_perf_ctxsw_result.returncode() == 0
        perf_ctxsw_is_enabled = true
        common_cflags += '-DHAVE_PERF_CTXSW'
        message('Enabling scheduler event support')
    else
        message('Disabling scheduler event support')
    endif
endif

libpthread_dep = cc.find_library('pthread', required: true)
libdl_dep = cc.find_library('dl', required: true)
librt_dep = cc.find_library('rt', required: true)

###########
# version #
###########

version_sh = find_program('misc/version.sh')
version_sh_cmd = '$(git describe --tags 2> /dev/null || echo v' + meson.project_version() + ')'
version_h = custom_target(
    'gen_version_h',
    output: 'version.h',
    command: [program_shell, '-c', version_sh.path() + ' @OUTPUT@' + ' ' + version_sh_cmd + ' .' + ' > /dev/null'],
)

#############################
# libmcount (arch specific) #
#############################

if arch == 'aarch64'
    libmcount_arch_include = include_directories('arch/aarch64')
    libmcount_arch_sources = [
        'arch/aarch64/cpuinfo.c',
        'arch/aarch64/mcount-dynamic.c',
        'arch/aarch64/mcount-insn.c',
        'arch/aarch64/mcount-support.c',
    ]
    libmcount_arch_asm_sources = [
        'arch/aarch64/dynamic.S',
        'arch/aarch64/mcount.S',
        'arch/aarch64/plthook.S',
    ]
elif arch == 'arm'
    libmcount_arch_include = include_directories('arch/arm')
    libmcount_arch_sources = [
        'arch/arm/cpuinfo.c',
        'arch/arm/mcount-support.c',
    ]
    libmcount_arch_asm_sources = [
        'arch/arm/mcount.S',
        'arch/arm/plthook.S',
    ]
elif arch == 'i386'
    libmcount_arch_include = include_directories('arch/i386')
    libmcount_arch_sources = [
        'arch/i386/cpuinfo.c',
        'arch/i386/mcount-dynamic.c',
        'arch/i386/mcount-support.c',
    ]
    libmcount_arch_asm_sources = [
        'arch/i386/common.S',
        'arch/i386/fentry.S',
        'arch/i386/mcount.S',
        'arch/i386/plthook.S',
    ]
elif arch == 'x86_64'
    libmcount_arch_include = include_directories('arch/x86_64')
    libmcount_arch_sources = [
        'arch/x86_64/cpuinfo.c',
        'arch/x86_64/mcount-dynamic.c',
        'arch/x86_64/mcount-event.c',
        'arch/x86_64/mcount-insn.c',
        'arch/x86_64/mcount-noplt.c',
        'arch/x86_64/mcount-support.c',
        'arch/x86_64/symbol.c',
    ]
    libmcount_arch_asm_sources = [
        'arch/x86_64/dynamic.S',
        'arch/x86_64/fentry.S',
        'arch/x86_64/mcount.S',
        'arch/x86_64/plthook.S',
        'arch/x86_64/xray.S',
    ]
else
    error('target architecture `@0@` is not supported'.format(arch))
endif

#############
# libmcount #
#############

libmcount_include = include_directories('libmcount')
libmcount_nop_sources = [
    'libmcount/mcount-nop.c',
]
libmcount_sources = [
    'libmcount/dynamic.c',
    'libmcount/event.c',
    'libmcount/mcount.c',
    'libmcount/misc.c',
    'libmcount/plthook.c',
    'libmcount/pmu.c',
    'libmcount/record.c',
    'libmcount/wrap.c',
    'utils/argspec.c',
    'utils/auto-args.c',
    'utils/debug.c',
    'utils/demangle.c',
    'utils/dwarf.c',
    'utils/filter.c',
    'utils/hashmap.c',
    'utils/rbtree.c',
    'utils/regs.c',
    'utils/script-luajit.c',
    'utils/script-python.c',
    'utils/script.c',
    'utils/symbol-libelf.c',
    'utils/symbol-rawelf.c',
    'utils/symbol.c',
    'utils/utils.c',
]

libmcount_base_deps = [
    cxa_demangle_dep,
    libcapstone_dep,
    libdl_dep,
    libdw_dep,
    libelf_dep,
    libluajit_dep,
    libncurses_dep,
    libpthread_dep,
    libpython3_dep,
    librt_dep,
]

libmcount = shared_library(
    'mcount', [libmcount_arch_sources, libmcount_arch_asm_sources, libmcount_sources, version_h],
    include_directories: [libmcount_arch_include, libmcount_include],
    dependencies: libmcount_base_deps,
    c_args: [common_cflags],
    install: true,
)

libmcount_fast = shared_library(
    'mcount-fast', [libmcount_arch_sources, libmcount_arch_asm_sources, libmcount_sources, version_h],
    include_directories: [libmcount_arch_include, libmcount_include],
    dependencies: libmcount_base_deps,
    c_args: [common_cflags, '-DDISABLE_MCOUNT_FILTER'],
    install: true,
)

libmcount_single = shared_library(
    'mcount-single', [libmcount_arch_sources, libmcount_arch_asm_sources, libmcount_sources, version_h],
    include_directories: [libmcount_arch_include, libmcount_include],
    dependencies: libmcount_base_deps,
    c_args: [common_cflags, '-DSINGLE_THREAD'],
    install: true,
)

libmcount_fast_single = shared_library(
    'mcount-fast-single', [libmcount_arch_sources, libmcount_arch_asm_sources, libmcount_sources, version_h],
    include_directories: [libmcount_arch_include, libmcount_include],
    dependencies: libmcount_base_deps,
    c_args: [common_cflags, '-DDISABLE_MCOUNT_FILTER', '-DSINGLE_THREAD'],
    install: true,
)

libmcount_fast_nop = shared_library(
    'mcount-nop', [libmcount_nop_sources, version_h],
    include_directories: [libmcount_include],
    dependencies: libmcount_base_deps,
    c_args: [common_cflags, '-DDISABLE_MCOUNT_FILTER', '-DSINGLE_THREAD'],
    install: true,
)

#################
# libtraceevent #
#################

libtraceevent_include = include_directories('libtraceevent')
libtraceevent_sources = [
    'libtraceevent/event-parse.c',
    'libtraceevent/event-plugin.c',
    'libtraceevent/kbuffer-parse.c',
    'libtraceevent/parse-filter.c',
    'libtraceevent/parse-utils.c',
    'libtraceevent/trace-seq.c',
]
libtraceevent_other_includes = include_directories('libtraceevent/include')

libtraceevent = static_library(
    'traceevent', [libtraceevent_sources],
    include_directories: [libtraceevent_include, libtraceevent_other_includes],
    c_args: [common_cflags],
    pic: true,
)

libtraceevent_dep = declare_dependency(
    include_directories: [libtraceevent_include],
    link_with: libtraceevent,
)

###########
# uftrace #
###########

uftrace_include = include_directories('cmds')
uftrace_sources = [
    'arch/' + arch + '/cpuinfo.c',
    'cmds/dump.c',
    'cmds/graph.c',
    'cmds/info.c',
    'cmds/live.c',
    'cmds/record.c',
    'cmds/recv.c',
    'cmds/replay.c',
    'cmds/report.c',
    'cmds/script.c',
    'cmds/tui.c',
    'uftrace.c',
    'utils/argspec.c',
    'utils/auto-args.c',
    'utils/data-file.c',
    'utils/debug.c',
    'utils/demangle.c',
    'utils/dwarf.c',
    'utils/extern.c',
    'utils/field.c',
    'utils/filter.c',
    'utils/fstack.c',
    'utils/graph.c',
    'utils/hashmap.c',
    'utils/kernel.c',
    'utils/pager.c',
    'utils/perf.c',
    'utils/rbtree.c',
    'utils/regs.c',
    'utils/report.c',
    'utils/script-luajit.c',
    'utils/script-python.c',
    'utils/script.c',
    'utils/session.c',
    'utils/symbol-libelf.c',
    'utils/symbol-rawelf.c',
    'utils/symbol.c',
    'utils/utils.c',
]
uftrace_deps = [
    cxa_demangle_dep,
    libdl_dep,
    libdw_dep,
    libelf_dep,
    libluajit_dep,
    libncurses_dep,
    libpthread_dep,
    libpython3_dep,
    librt_dep,
    libtraceevent_dep,
]

uftrace = executable(
    'uftrace', [uftrace_sources],
    include_directories: [uftrace_include],
    dependencies: uftrace_deps,
    c_args: [common_cflags],
    install: true,
)

###########
# dbginfo #
###########

dbginfo_sources = [
    'misc/dbginfo.c',
    'utils/argspec.c',
    'utils/auto-args.c',
    'utils/debug.c',
    'utils/demangle.c',
    'utils/dwarf.c',
    'utils/filter.c',
    'utils/rbtree.c',
    'utils/regs.c',
    'utils/symbol-libelf.c',
    'utils/symbol-rawelf.c',
    'utils/symbol.c',
    'utils/utils.c',
]
dbginfo_deps = [
    cxa_demangle_dep,
    libdl_dep,
    libdw_dep,
    libelf_dep,
]

if get_option('build-dbginfo').enabled()
    dbginfo = executable(
        'dbginfo', [dbginfo_sources],
        dependencies: dbginfo_deps,
        c_args: [common_cflags],
    )
endif

#############
# demangler #
#############

demangler_sources = [
    'misc/demangler.c',
    'utils/demangle.c',
    'utils/debug.c',
    'utils/utils.c',
]
demangler_deps = [
    cxa_demangle_dep,
]

if get_option('build-demangler').enabled()
    demangler = executable(
        'demangler', [demangler_sources],
        dependencies: demangler_deps,
        c_args: [common_cflags],
    )
endif

#########
# tests #
#########

unittests_includes = include_directories('tests', 'arch/' + arch)
unittests_sources = [
    'cmds/dump.c',
    'cmds/graph.c',
    'cmds/info.c',
    'cmds/live.c',
    'cmds/record.c',
    'cmds/recv.c',
    'cmds/replay.c',
    'cmds/report.c',
    'cmds/script.c',
    'cmds/tui.c',
    'tests/unittest.c',
    'uftrace.c',
    'utils/argspec.c',
    'utils/auto-args.c',
    'utils/data-file.c',
    'utils/debug.c',
    'utils/demangle.c',
    'utils/dwarf.c',
    'utils/extern.c',
    'utils/field.c',
    'utils/filter.c',
    'utils/fstack.c',
    'utils/graph.c',
    'utils/hashmap.c',
    'utils/kernel.c',
    'utils/pager.c',
    'utils/perf.c',
    'utils/rbtree.c',
    'utils/regs.c',
    'utils/report.c',
    'utils/script-luajit.c',
    'utils/script-python.c',
    'utils/script.c',
    'utils/session.c',
    'utils/symbol-libelf.c',
    'utils/symbol-rawelf.c',
    'utils/symbol.c',
    'utils/utils.c',
    libmcount_arch_sources,
    libmcount_sources,
]
unittests_deps = [
    cxa_demangle_dep,
    libcapstone_dep,
    libdl_dep,
    libdw_dep,
    libelf_dep,
    libluajit_dep,
    libncurses_dep,
    libpthread_dep,
    libpython3_dep,
    librt_dep,
    libtraceevent_dep,
]
tests_dir = meson.source_root() + '/tests'
tests_unittest_h = tests_dir + '/unittest.h'

if get_option('with-tests').enabled()
    unittests = executable(
        'unittests', [unittests_sources],
        include_directories: [unittests_includes, libmcount_arch_include, libmcount_include],
        dependencies: unittests_deps,
        c_args: [common_cflags, '-DUNIT_TEST', '-include', tests_unittest_h],
    )

    run_target('check-unit',
        command: [unittests],
        depends: [unittests],
    )

    run_target('check-uftrace',
        command: ['sh', '-c', 'cd @0@ && objdir=@1@ ./runtest.py'.format(tests_dir, meson.current_build_dir())],
        depends: [
            libmcount,
            libmcount_fast,
            libmcount_single,
            libmcount_fast_single,
            libmcount_fast_nop,
            uftrace,
        ],
    )
else
    run_target('check-unit',
        command: ['echo', 'Test not enabled, please reconfigure with -Dwith-test=enabled option.'],
    )

    run_target('check-uftrace',
        command: ['echo', 'Tests not enabled, please reconfigure with -Dwith-test=enabled option.'],
    )
endif

#################
# documentation #
#################

doc_sources = [
    'uftrace-dump',
    'uftrace-graph',
    'uftrace-info',
    'uftrace-live',
    'uftrace-record',
    'uftrace-recv',
    'uftrace-replay',
    'uftrace-report',
    'uftrace-script',
    'uftrace-tui',
    'uftrace',
]

if get_option('without-doc').disabled()
    program_pandoc = find_program('pandoc', required: true)

    foreach source_file : doc_sources
        if get_option('doc-lang') == 'en'
            markdown_file = files('doc/@0@.md'.format(source_file))
        elif get_option('doc-lang') == 'ko'
            markdown_file = files('doc/ko/@0@.md'.format(source_file))
        endif

        man_page = '@0@.1'.format(source_file)
        man_page_target = run_command('sh', '-c', 'printf @0@ | sed \'s/[-|\/]/_/g\''.format(man_page)).stdout().strip()

        custom_target(
            man_page_target,
            command: [program_pandoc, '-s', '@INPUT@', '-t', 'man', '-o', '@OUTPUT@'],
            input: markdown_file,
            output: man_page,
            install: true,
            install_dir: join_paths(get_option('mandir'), 'man1'),
        )
    endforeach
endif

#########
# ctags #
#########

ctags_command = ''
ctags_command += 'cd @0@ && '.format(meson.source_root())
ctags_command += 'find . -name "*\.[chS]" -o -path ./tests -prune -o -path ./check-deps -prune | '
ctags_command += 'xargs ctags --regex-asm=\'/^(GLOBAL|ENTRY|END)\(([^)]*)\).*/\2/\''

run_target('ctags',
    command: ['sh', '-c', ctags_command],
)

###########
# summary #
###########

color_green = '\u001b[32m'
color_red = '\u001b[31m'
color_off = '\u001b[0m'

text_on = color_green + 'on' + color_off + ' '
text_off = color_red + 'off' + color_off

libcapstone_is_enabled  = (libcapstone_is_enabled)  ? text_on : text_off
libdw_is_enabled        = (libdw_is_enabled)        ? text_on : text_off
libelf_is_enabled       = (libelf_is_enabled)       ? text_on : text_off
libluajit_is_enabled    = (libluajit_is_enabled)    ? text_on : text_off
libpython3_is_enabled   = (libpython3_is_enabled)   ? text_on : text_off
libncurses_is_enabled   = (libncurses_is_enabled)   ? text_on : text_off
cxa_demangle_is_enabled = (cxa_demangle_is_enabled) ? text_on : text_off
perf_clockid_is_enabled = (perf_clockid_is_enabled) ? text_on : text_off
perf_ctxsw_is_enabled   = (perf_ctxsw_is_enabled)   ? text_on : text_off

message('          prefix: @0@'.format(get_option('prefix')))
message('     libcapstone: [ @0@ ] - full dynamic tracing support'.format(libcapstone_is_enabled))
message('           libdw: [ @0@ ] - dwarf support'.format(libdw_is_enabled))
message('          libelf: [ @0@ ] - more flexible ELF data handling'.format(libelf_is_enabled))
message('       libluajit: [ @0@ ] - luajit scripting support'.format(libluajit_is_enabled))
message('       libpython: [ @0@ ] - python scripting support'.format(libpython3_is_enabled))
message('      libncurses: [ @0@ ] - terminal UI support'.format(libncurses_is_enabled))
message('    cxa_demangle: [ @0@ ] - full demangler support with libstdc++'.format(cxa_demangle_is_enabled))
message('      perf event: [ @0@ ] - perf event support'.format(perf_clockid_is_enabled))
message('   perf schedule: [ @0@ ] - perf scheduler event support'.format(perf_ctxsw_is_enabled))
