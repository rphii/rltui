project('rltui', 'c', meson_version: '>=1.1', version: '9999')

sources = [
  'rltui/tui-die.c',
  'rltui/tui-global.c',
  'rltui/tui-input.c',
  'rltui/tui-raw.c',
  'rltui/tui.c',
  ]

headers = [
  'rltui/tui-die.h',
  'rltui/tui-esc-code.h',
  'rltui/tui-global.h',
  'rltui/tui-input.h',
  'rltui/tui-point.h',
  'rltui/tui-raw.h',
  'rltui/tui-rect.h',
  'rltui/tui-write.h',
  'rltui/tui.h',
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


