// Theme token sets — ported VERBATIM from the OP4 design handoff
// (docs/plans/design_handoff_op4_synth/OP4 App.dc.html, `THEMES` + `ALGACC`).
// This is the canonical visual identity — do not re-derive these values.

export type ThemeName = 'DX21' | 'TX81Z' | 'SK-1'

/** The ~30 named tokens, redefined per theme. Applied as CSS custom properties. */
export type TokenSet = Record<string, string>

export const THEMES: Record<ThemeName, TokenSet> = {
  'DX21': {
    '--chassis': 'linear-gradient(180deg,#37342f,#2b2825)', '--chassis-b': '#46423b',
    '--head': 'linear-gradient(180deg,#3d3a34,#312e29)', '--head-b': '#191713',
    '--panel': '#262421', '--panel-b': '#3d3a35', '--inset': '#1e1c19', '--dash': '#57523f',
    '--ink': '#d8d2c2', '--silk': '#96907f', '--faint': '#6e6959', '--val': '#c4beb0',
    '--disp': 'linear-gradient(180deg,#cdd964,#b6c74b)', '--disp-ink': '#26310f', '--disp-dim': '#5a6630', '--disp-b': '#8a9a35', '--disp-dot': '#c3d254', '--disp-track': 'rgba(38,49,15,.2)', '--disp-inset': 'inset 0 1px 4px rgba(0,0,0,.3)',
    '--key': 'linear-gradient(180deg,#ddd6c6,#c8c0ad)', '--key-b': '#a89f8a', '--key-ink': '#33312e',
    '--keyA': 'linear-gradient(180deg,#a9c25c,#8fae4a)', '--keyA-b': '#77913a', '--keyA-ink': '#1f2912',
    '--knob': 'radial-gradient(circle at 35% 30%,#45423c,#2a2825 70%)', '--knob-b': '#524e46', '--ptr': '#e8e2d2',
    '--acc': '#a9c25c', '--accg': 'rgba(169,194,92,.8)',
    '--szl': '#282b1e', '--szl-b': '#4d5433', '--szl-t': '#a9c25c',
    '--nav': 'linear-gradient(180deg,#ddd6c6,#c8c0ad)', '--nav-b': '#a89f8a', '--nav-ink': '#33312e',
  },
  'TX81Z': {
    '--chassis': 'linear-gradient(180deg,#202124,#18191b)', '--chassis-b': '#303236',
    '--head': 'linear-gradient(180deg,#232427,#1b1c1e)', '--head-b': '#0a0a0b',
    '--panel': '#141518', '--panel-b': '#2c2e32', '--inset': '#0e0f11', '--dash': '#3a3d43',
    '--ink': '#e4e2dd', '--silk': '#84878d', '--faint': '#55585e', '--val': '#c9ccd1',
    '--disp': '#120705', '--disp-ink': '#ff6a3d', '--disp-dim': '#8a3a20', '--disp-b': '#38160c', '--disp-dot': '#17191c', '--disp-track': 'rgba(255,106,61,.18)', '--disp-inset': 'inset 0 2px 8px rgba(0,0,0,.8)',
    '--key': 'linear-gradient(180deg,#2b2d31,#1f2124)', '--key-b': '#3a3c41', '--key-ink': '#c9ccd1',
    '--keyA': '#170d09', '--keyA-b': '#58260f', '--keyA-ink': '#ff6a3d',
    '--knob': 'radial-gradient(circle at 35% 30%,#2f3135,#1a1b1d 70%)', '--knob-b': '#383a3f', '--ptr': '#e4e2dd',
    '--acc': '#ff6a3d', '--accg': 'rgba(255,106,61,.8)',
    '--szl': '#1c0f08', '--szl-b': '#58260f', '--szl-t': '#ff6a3d',
    '--nav': 'linear-gradient(180deg,#2b2d31,#1f2124)', '--nav-b': '#3a3c41', '--nav-ink': '#c9ccd1',
  },
  'SK-1': {
    '--chassis': 'linear-gradient(180deg,#ece8e0,#dad4c9)', '--chassis-b': '#b8b2a6',
    '--head': 'linear-gradient(180deg,#f2efe8,#e2ddd3)', '--head-b': '#c5bfb4',
    '--panel': '#f7f5f0', '--panel-b': '#cfc9be', '--inset': '#e6e2d9', '--dash': '#b8b2a6',
    '--ink': '#33312e', '--silk': '#8a8478', '--faint': '#a39d90', '--val': '#55514a',
    '--disp': 'linear-gradient(180deg,#8fdfd8,#6fd2c9)', '--disp-ink': '#0e3532', '--disp-dim': '#2f6b64', '--disp-b': '#3aa89e', '--disp-dot': '#7fd9d2', '--disp-track': 'rgba(14,53,50,.18)', '--disp-inset': 'inset 0 1px 4px rgba(0,0,0,.2)',
    '--key': 'linear-gradient(180deg,#fbfaf7,#e9e5de)', '--key-b': '#c5bfb4', '--key-ink': '#4a463e',
    '--keyA': 'linear-gradient(180deg,#f48fb1,#ec6a99)', '--keyA-b': '#d84f7f', '--keyA-ink': '#4a1228',
    '--knob': 'radial-gradient(circle at 35% 30%,#4a4f55,#33373c 70%)', '--knob-b': '#2b2f34', '--ptr': '#f7f5f0',
    '--acc': '#ec6a99', '--accg': 'rgba(236,106,153,.8)',
    '--szl': '#fbe7ef', '--szl-b': '#e9a8c2', '--szl-t': '#d84f7f',
    '--nav': 'linear-gradient(180deg,#fbfaf7,#e9e5de)', '--nav-b': '#c5bfb4', '--nav-ink': '#4a463e',
  },
}

/** Glyph-only accents per theme (mirror of display ink / base) for the algorithm diagrams. */
export interface AlgAccent {
  base: string; ac: string; carInk: string; numUnsel: string
  selBg: string; selB: string; unselBg: string; unselB: string; glow: string
}

export const ALGACC: Record<ThemeName, AlgAccent> = {
  'DX21': { base: '#57523f', ac: '#26310f', carInk: '#c3d254', numUnsel: '#57523f', selBg: 'linear-gradient(180deg,#cdd964,#b3c443)', selB: '#8a9a35', unselBg: 'linear-gradient(180deg,#ddd6c6,#c8c0ad)', unselB: '#a89f8a', glow: 'none' },
  'TX81Z': { base: '#4d4f54', ac: '#ff6a3d', carInk: '#170d09', numUnsel: '#84878d', selBg: '#170d09', selB: '#ff6a3d55', unselBg: 'linear-gradient(180deg,#26272a,#1c1d20)', unselB: '#34363a', glow: '0 0 12px rgba(255,106,61,.2)' },
  'SK-1': { base: '#8a8478', ac: '#0e3532', carInk: '#7fd9d2', numUnsel: '#8a8478', selBg: 'linear-gradient(180deg,#8fdfd8,#6fd2c9)', selB: '#3aa89e', unselBg: 'linear-gradient(180deg,#fbfaf7,#e9e5de)', unselB: '#c5bfb4', glow: 'none' },
}

export const THEME_NAMES: ThemeName[] = ['DX21', 'TX81Z', 'SK-1']
