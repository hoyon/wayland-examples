project('wayland-examples', 'cpp', 'c',
       default_options : ['c_std=c11', 'cpp_std=c++17'])

wl_client_dep = dependency('wayland-client')

wl_protocols_dep = dependency('wayland-protocols')
wl_protocols_dir = wl_protocols_dep.get_pkgconfig_variable('pkgdatadir')

wl_scanner_dep = dependency('wayland-scanner')
wl_scanner_path = wl_scanner_dep.get_pkgconfig_variable('wayland_scanner')
wl_scanner = find_program(wl_scanner_path)

wl_scanner_header = generator(wl_scanner,
                              output : '@BASENAME@.h',
                              arguments : ['client-header', '@INPUT@', '@OUTPUT@']
                             )

wl_scanner_code = generator(wl_scanner,
                            output : '@BASENAME@.c',
                            arguments : ['private-code', '@INPUT@', '@OUTPUT@']
                           )

rt_dep = meson.get_compiler('c').find_library('rt')
deps = [wl_client_dep, rt_dep]
c_flags = ['-Wall', '-Wextra']

xdg_shell_stable = wl_protocols_dir + '/stable/xdg-shell/xdg-shell.xml'
xdg_shell_unstable = wl_protocols_dir + '/unstable/xdg-shell/xdg-shell-unstable-v6.xml'

examples = {
    '01-wl-shell': {
        'files': ['wl-shell.cpp'],
        'protocols': []
    },
    '02-xdg-shell-unstable': {
        'files': ['xdg-shell-unstable.cpp'],
        'protocols': [xdg_shell_unstable]
    },
    '03-xdg-shell-stable': {
        'files': ['xdg-shell-stable.cpp'],
        'protocols': [xdg_shell_stable]
    },
}

foreach name, params : examples
    protocol_files = []
    foreach protocol : params.get('protocols', [])
        header = wl_scanner_header.process(protocol)
        code = wl_scanner_code.process(protocol)
        protocol_files += [header, code]
    endforeach

    paths = []
    foreach file : params.get('files')
        paths += name + '/' + file
    endforeach

    executable(name, paths, protocol_files, dependencies: deps, c_args: c_flags)
endforeach
