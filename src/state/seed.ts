// Seed data — transcribed from OP4 App.dc.html renderVals(). Reference only,
// but faithful so the UI matches the reference screenshots out of the box.
import type { Operator, Patch } from './types'

// ---------- VOICE: the initial patch (B12 NITE PAD) ----------
const OPS_SEED: Operator[] = [
  { name: 'OP 1', role: 'CARRIER', carrier: true, ratio: '1.00', lvl: '99', det: '+0', wave: 'W1 SIN', fix: false, eg: '31·18·06·24', ep: { a: 0.04, d1: 0.5, s: 0.62, r: 0.4 } },
  { name: 'OP 2', role: 'MOD', carrier: false, ratio: '14.00', lvl: '58', det: '+2', wave: 'W1 SIN', fix: false, eg: '31·26·00·31', ep: { a: 0.02, d1: 0.18, s: 0.1, r: 0.15 } },
  { name: 'OP 3', role: 'MOD', carrier: false, ratio: '1.00', lvl: '66', det: '−4', wave: 'W2 SQR', fix: false, eg: '28·14·10·20', ep: { a: 0.1, d1: 0.42, s: 0.45, r: 0.35 } },
  { name: 'OP 4', role: 'MOD', carrier: false, ratio: '5.00', lvl: '41', det: '+0', wave: 'W3 TRI', fix: false, eg: '31·30·00·28', ep: { a: 0.02, d1: 0.1, s: 0.05, r: 0.1 } },
]

export const INITIAL_PATCH: Patch = {
  bank: 'B12',
  slot: 'B12',
  name: 'NITE PAD',
  edited: true,
  algorithm: 2, // glyph #3 (0-indexed) is selected in the reference
  feedback: 6,
  ops: OPS_SEED,
  lfo: {
    wave: 'TRI', sync: true,
    knobs: [
      { key: 'speed', label: 'SPEED', val: 12 },
      { key: 'delay', label: 'DELAY', val: 30 },
      { key: 'pmd', label: 'PMD', val: 41 },
      { key: 'amd', label: 'AMD', val: 8 },
    ],
  },
  pitchEg: { label: 'R 72 · L 50' },
  fn: { voice: 'POLY', porta: 42, transpose: -12, assign: 'DUAL' },
  sizzler: { amount: 35, mode: 'RND', targets: 9, freeze: false },
}

// ---------- ALGORITHM TOPOLOGIES ----------
// The canonical Yamaha 4-op algorithm set (shared by DX21/DX27/DX100/TX81Z).
// Feedback is always on OP4; carriers are the operators that reach the output.
// Verified against the TX81Z connection matrix in gesellkammer/sc-tx81z
// (ref/tx81z-algorithms.jpg + the Csound `kALG` matrix). Edge = [modulator, carrier].
export interface AlgTopology {
  nodes: Record<number, [number, number]>
  edges: [number, number][]
  carriers: number[]
}
export const ALGS: AlgTopology[] = [
  // 1: 4→3→2→1
  { nodes: { 1: [0, 0], 2: [0, 1], 3: [0, 2], 4: [0, 3] }, edges: [[4, 3], [3, 2], [2, 1]], carriers: [1] },
  // 2: (3,4)→2→1
  { nodes: { 1: [0.5, 0], 2: [0.5, 1], 3: [0, 2], 4: [1, 2] }, edges: [[3, 2], [4, 2], [2, 1]], carriers: [1] },
  // 3: 3→2→1 and 4→1  (carrier 1)
  { nodes: { 1: [0.5, 0], 2: [0, 1], 4: [1, 1], 3: [0, 2] }, edges: [[2, 1], [4, 1], [3, 2]], carriers: [1] },
  // 4: 2→1 and 4→3→1  (carrier 1)
  { nodes: { 1: [0.5, 0], 2: [0, 1], 3: [1, 1], 4: [1, 2] }, edges: [[2, 1], [3, 1], [4, 3]], carriers: [1] },
  // 5: 2→1 ∥ 4→3  (two carriers)
  { nodes: { 1: [0, 0], 2: [0, 1], 3: [1, 0], 4: [1, 1] }, edges: [[2, 1], [4, 3]], carriers: [1, 3] },
  // 6: 4→(1,2,3)  (three carriers, one modulator)
  { nodes: { 1: [0, 0], 2: [1, 0], 3: [2, 0], 4: [1, 1] }, edges: [[4, 1], [4, 2], [4, 3]], carriers: [1, 2, 3] },
  // 7: 4→3, with 1 & 2 as standalone carriers
  { nodes: { 1: [0, 0], 2: [1, 0], 3: [2, 0], 4: [2, 1] }, edges: [[4, 3]], carriers: [1, 2, 3] },
  // 8: 1 ∥ 2 ∥ 3 ∥ 4  (additive — four carriers)
  { nodes: { 1: [0, 0], 2: [1, 0], 3: [2, 0], 4: [3, 0] }, edges: [], carriers: [1, 2, 3, 4] },
]

// ---------- LIBRARIAN ----------
export const CATS: Record<string, string> = { EP: 'var(--acc)', BASS: '#c2704a', BRASS: '#d0b45a', PAD: '#9aa3ad', LEAD: 'var(--acc)', SFX: '#b08bb5', ORG: '#7fa8a0', STR: 'var(--val)', PERC: '#c2704a', KEY: 'var(--acc)' }
export const BANK_NAMES = ['GLASS EP', 'FULLTINES', 'LATELY BASS', 'SOLID BASS', 'BRASS 1', 'SAX BC1', 'GLASS EP', 'NITE PAD', 'WARM STR', 'CHIME PAD', 'SYN LEAD', 'SQUARE LD', 'PIPE ORG', 'ROCK ORG', 'CLAV 1', 'HARPSI', 'MARIMBA', 'VIBES', 'STEEL DRM', 'TIMPANI', 'WOOD BASS', 'FRETLESS', 'HORN SECT', 'TRUMPET', 'ANLG PAD', 'ICE FIELD', 'ZAP SFX', 'STARSHIP', 'BELL TREE', 'TUB BELLS', '', '']
export const BANK_CATSEQ = ['EP', 'EP', 'BASS', 'BASS', 'BRASS', 'BRASS', 'EP', 'PAD', 'STR', 'PAD', 'LEAD', 'LEAD', 'ORG', 'ORG', 'KEY', 'KEY', 'PERC', 'PERC', 'PERC', 'PERC', 'BASS', 'BASS', 'BRASS', 'BRASS', 'PAD', 'PAD', 'SFX', 'SFX', 'PERC', 'PERC', '', '']
export const BANKS: [string, string, number][] = [['FACTORY A', '32', 1], ['E.PIANO COLLECTION', '32', 0], ['USER 01', '27', 0], ['TX ROM 1', '32', 0], ['GEN · SCI-FI 06', '18', 0], ['RX21 KITS', '12', 0]]
export const LIB_TAGS = ['ALL', 'EP', 'BASS', 'PAD', 'LEAD', 'BRASS', 'SFX']

// ---------- DRUMS ----------
export const GRP: Record<string, string> = { KICKS: '#c2704a', SNARES: 'var(--acc)', HATS: '#d0b45a', TOMS: '#9aa3ad', CYMB: '#b08bb5', PERC: 'var(--acc)' }
// [n, name, key, level, pan, tune, group, choke, mute, solo]
export const DRUMS: [string, string, string, number, string, string, string, string, number, number][] = [
  ['01', 'BD TIGHT', 'C1', 92, 'C', '+0', 'KICKS', '', 0, 0], ['02', 'BD ROOM', 'C#1', 84, 'C', '−3', 'KICKS', '', 0, 0],
  ['03', 'SD CRACK', 'D1', 88, 'R4', '+0', 'SNARES', '', 0, 1], ['04', 'SD RIM', 'D#1', 70, 'L6', '+2', 'SNARES', '', 0, 0],
  ['05', 'HH CLOSED', 'F#1', 76, 'L12', '+0', 'HATS', 'A', 0, 0], ['06', 'HH OPEN', 'A#1', 74, 'L12', '+0', 'HATS', 'A', 0, 0],
  ['07', 'TOM LO', 'F1', 81, 'R10', '−5', 'TOMS', '', 1, 0], ['08', 'TOM HI', 'G1', 81, 'L10', '+4', 'TOMS', '', 1, 0],
  ['09', 'CRASH', 'C#2', 66, 'R16', '+0', 'CYMB', '', 0, 0], ['10', 'COWBELL', 'G#2', 58, 'R20', '+0', 'PERC', '', 0, 0],
]
export const DRUM_KEYMAP: Record<number, string> = { 0: 'KICKS', 1: 'KICKS', 2: 'SNARES', 3: 'SNARES', 5: 'TOMS', 6: 'HATS', 7: 'TOMS', 10: 'HATS', 13: 'CYMB', 20: 'PERC' }

// ---------- FX ----------
// [name, enabled, knobs[[label, val]]]
export const FX_BLOCKS: [string, number, [string, number][]][] = [
  ['COMP', 1, [['THRSH', 40], ['RATIO', 60], ['GAIN', 55]]],
  ['DIST', 0, [['DRIVE', 30], ['TONE', 50], ['LVL', 62]]],
  ['EQ', 1, [['LOW', 55], ['MID', 48], ['HIGH', 66], ['Q', 40]]],
  ['PITCH', 1, [['SHIFT', 52], ['FINE', 50], ['MIX', 44]]],
  ['DELAY', 1, [['TIME', 58], ['FB', 46], ['MIX', 40]]],
]
export const SPX_PROGS = ['REVERB HALL', 'REVERB ROOM', 'GATE REVERB', 'EARLY REF', 'STEREO ECHO', 'SYMPHONIC']
export const SPX_PARAMS: [string, number][] = [['REV TIME', 62], ['HI RATIO', 48], ['DELAY', 40], ['DENSITY', 70], ['MIX', 45]]

// ---------- GENERATOR ----------
export const GEN_CATS = ['PADS', 'SCI-FI', 'RPG', 'PERCUSSION', 'GM', 'LEADS']
// [name, kept]
export const GEN_NAMES: [string, number][] = [['NEBULA DRIFT', 1], ['WARP CORE', 0], ['ANDROID DRM', 1], ['ZERO-G PAD', 0], ['PULSAR SEQ', 0], ['ION STORM', 1], ['CRYO SWEEP', 0], ['VOID BELL', 0]]

// ---------- MIDI ----------
// [param, cc, assigned]
export const CC_ROWS: [string, string, number][] = [['OP1 LEVEL', '16', 1], ['OP2 LEVEL', '17', 1], ['OP3 LEVEL', '18', 1], ['OP4 LEVEL', '19', 1], ['FEEDBACK', '20', 1], ['LFO SPEED', '21', 1], ['LFO PMD', '22', 0], ['ALGORITHM', '—', 0], ['PORTA TIME', '5', 1], ['SIZZLER AMT', '70', 1]]
export const NRPN_ROWS: [string, string][] = [['ALGORITHM', '00 07'], ['FEEDBACK', '00 08'], ['OP1 RATIO', '01 00'], ['OP1 LEVEL', '01 01'], ['OP1 EG ATK', '01 02'], ['LFO SPEED', '00 10'], ['PITCH EG R1', '00 20'], ['WAVE SELECT', '01 0A']]

// ---------- NAV ----------
export const NAV: [string, string][] = [['voice', 'VOICE'], ['lib', 'LIBRARY'], ['drums', 'DRUMS'], ['fx', 'FX'], ['gen', 'GEN'], ['midi', 'MIDI']]
export const TITLES: Record<string, string> = { voice: 'FM VOICE EDITOR', lib: 'LIBRARIAN', drums: 'DRUM RACK', fx: 'EFFECTS', gen: 'GENERATOR', midi: 'MIDI / SETTINGS' }
