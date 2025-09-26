
sources = [
  'tui-die.c',
  'tui-global.c',
  'tui-input.c',
  'tui-raw.c',
  'tui.c',
  ]

headers = [
  'tui-die.h',
  'tui-esc-code.h',
  'tui-global.h',
  'tui-input.h',
  'tui-point.h',
  'tui-raw.h',
  'tui-rect.h',
  'tui-write.h',
  'tui.h',
  ]

rlc_dep = dependency('rlc', fallback : ['rlc', 'rlc_dep'], default_options: ['default_library=static'])
rlso_dep = dependency('rlso', fallback : ['rlso', 'rlso_dep'], default_options: ['default_library=static'])

install_headers(headers, subdir: 'rltui')
install_headers('rltui.h')

librltui = library('rltui',
  sources,
  dependencies: [rlc_dep, rlso_dep],
  install: true,
  )

pkg = import('pkgconfig')
pkg.generate(
  libraries: librltui,
  version: '9999',
  name: 'rltui',
  description: 'TUI helpers for rl*',
  )


rltui_dep = declare_dependency(
  link_with: librltui,
  include_directories: '.',
  )


