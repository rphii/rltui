project('rltui', 'c', meson_version: '>=1.1', version: '0.0.3')

sources = [
  'rltui/tui-buffer.c',
  'rltui/tui-cell.c',
  'rltui/tui-color.c',
  'rltui/tui-die.c',
  'rltui/tui-fx.c',
  'rltui/tui-global.c',
  'rltui/tui-input.c',
  'rltui/tui-point.c',
  'rltui/tui-raw.c',
  'rltui/tui-rect.c',
  'rltui/tui-screen.c',
  'rltui/tui-sync.c',
  'rltui/tui-text.c',
  'rltui/tui.c',
  ]

headers = [
  'rltui/tui-buffer.h',
  'rltui/tui-cell.h',
  'rltui/tui-color.h',
  'rltui/tui-die.h',
  'rltui/tui-esc-code.h',
  'rltui/tui-fx.h',
  'rltui/tui-global.h',
  'rltui/tui-input.h',
  'rltui/tui-point.h',
  'rltui/tui-range.h',
  'rltui/tui-raw.h',
  'rltui/tui-rect.h',
  'rltui/tui-screen.h',
  'rltui/tui-sync.h',
  'rltui/tui-text.h',
  'rltui/tui-write.h',
  'rltui/tui.h',
  ]

rlc_dep = dependency('rlc', fallback : ['rlc', 'rlc_dep'], default_options: ['default_library=static'])
rlso_dep = dependency('rlso', fallback : ['rlso', 'rlso_dep'], default_options: ['default_library=static'])
rlwcwidth_dep = dependency('rlwcwidth', fallback : ['rlwcwidth', 'rlwcwidth_dep'], default_options: ['default_library=static'])

install_headers(headers, subdir: 'rltui')
install_headers('rltui.h')

librltui = library('rltui',
  sources,
  dependencies: [rlc_dep, rlso_dep, rlwcwidth_dep],
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


