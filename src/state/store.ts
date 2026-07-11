import { create } from 'zustand'
import type { Envelope, Operator, Patch, SynthMode, ViewId } from './types'
import type { ThemeName } from '../theme/themes'
import {
  INITIAL_PATCH, FX_BLOCKS, SPX_PARAMS, GEN_NAMES, DRUMS, CC_ROWS,
} from './seed'

// ---- mutable per-view slices ----
export interface FxBlock { name: string; enabled: boolean; knobs: { label: string; val: number }[] }
export interface SpxState { program: number; params: { label: string; val: number }[] }
export interface GenState { category: number; count: number; mutation: number; kept: boolean[] }
export interface DrumRow {
  n: string; name: string; key: string; lvl: number; pan: string; tune: string
  grp: string; choke: string; mute: boolean; solo: boolean
}
export interface CcRow { param: string; cc: string; assigned: boolean }
export interface MidiState {
  device: number; echo: boolean; channel: number
  engine: 'VINTAGE' | 'MODERN'; oversample: '1×' | '2×' | '4×'
  learnMode: boolean; learningRow: number | null; ccRows: CcRow[]
}

interface Snapshot {
  patch: Patch
  fx: FxBlock[]
  spx: SpxState
  gen: GenState
  drums: DrumRow[]
  midi: MidiState
}

interface Store extends Snapshot {
  // UI
  view: ViewId
  mode: SynthMode
  theme: ThemeName
  hardwareLink: boolean
  // loaded VMEM bank (from the plugin); empty in the browser demo
  bank: { loaded: boolean; current: number; names: string[] }
  // undo
  past: Snapshot[]
  interacting: boolean
  _pending: Snapshot | null

  // UI actions (not undoable)
  setBank: (b: { loaded: boolean; current: number; names: string[] }) => void
  setView: (v: ViewId) => void
  setMode: (m: SynthMode) => void
  setTheme: (t: ThemeName) => void
  toggleHardwareLink: () => void

  // gesture coalescing — one drag = one undo entry
  beginInteraction: () => void
  endInteraction: () => void

  // undoable mutations
  setAlgorithm: (i: number) => void
  setFeedback: (v: number) => void
  setOpField: (opIdx: number, field: keyof Operator, value: string) => void
  setOpEnvelope: (opIdx: number, ep: Envelope) => void
  setLfoKnob: (idx: number, val: number) => void
  setFxKnob: (blockIdx: number, knobIdx: number, val: number) => void
  toggleFxBlock: (blockIdx: number) => void
  setSpxProgram: (i: number) => void
  setSpxParam: (i: number, val: number) => void
  setGenField: (field: 'category' | 'count' | 'mutation', value: number) => void
  toggleKeep: (i: number) => void
  toggleDrumMute: (i: number) => void
  toggleDrumSolo: (i: number) => void
  setDrumLevel: (i: number, val: number) => void
  setMidiField: <K extends keyof MidiState>(field: K, value: MidiState[K]) => void
  startLearn: (rowIdx: number) => void
  clearCc: (rowIdx: number) => void

  undo: () => void
  canUndo: () => boolean

  // audio / MIDI I/O — stubbed this pass (see plan known-open-items)
  audioStub: (action: string) => void
}

const initialFx: FxBlock[] = FX_BLOCKS.map(([name, enabled, knobs]) => ({
  name, enabled: !!enabled, knobs: knobs.map(([label, val]) => ({ label, val })),
}))
const initialSpx: SpxState = { program: 0, params: SPX_PARAMS.map(([label, val]) => ({ label, val })) }
const initialGen: GenState = { category: 1, count: 8, mutation: 64, kept: GEN_NAMES.map(([, k]) => !!k) }
const initialDrums: DrumRow[] = DRUMS.map((d) => ({
  n: d[0], name: d[1], key: d[2], lvl: d[3], pan: d[4], tune: d[5], grp: d[6], choke: d[7], mute: !!d[8], solo: !!d[9],
}))
const initialMidi: MidiState = {
  device: 1, echo: true, channel: 1, engine: 'VINTAGE', oversample: '2×',
  learnMode: true, learningRow: 6,
  ccRows: CC_ROWS.map(([param, cc, assigned]) => ({ param, cc, assigned: !!assigned })),
}

const clone = <T,>(o: T): T => JSON.parse(JSON.stringify(o))

export const useStore = create<Store>((set, get) => {
  const snapshot = (): Snapshot => {
    const s = get()
    return clone({ patch: s.patch, fx: s.fx, spx: s.spx, gen: s.gen, drums: s.drums, midi: s.midi })
  }
  /**
   * Snapshot current state for undo, then apply `mutate` to a draft snapshot.
   * During an interaction (a drag), the whole gesture collapses to a single
   * undo entry: the pre-gesture snapshot is pushed once, on the first change.
   */
  const commit = (mutate: (draft: Snapshot) => void) => {
    const before = snapshot()
    const draft = clone(before)
    mutate(draft)
    const { interacting, _pending, past } = get()
    if (interacting) {
      if (_pending) set({ ...draft, past: [...past, _pending], _pending: null })
      else set({ ...draft })
    } else {
      set({ ...draft, past: [...past, before] })
    }
  }

  return {
    patch: clone(INITIAL_PATCH),
    fx: initialFx,
    spx: initialSpx,
    gen: initialGen,
    drums: initialDrums,
    midi: initialMidi,

    bank: { loaded: false, current: 0, names: [] },
    view: 'voice',
    mode: 'DX21',
    theme: 'DX21',
    hardwareLink: true,
    past: [],
    interacting: false,
    _pending: null,

    setBank: (b) => set({ bank: b }),
    setView: (v) => set({ view: v }),
    setMode: (m) => set({ mode: m }),
    setTheme: (t) => set({ theme: t }),
    toggleHardwareLink: () => set({ hardwareLink: !get().hardwareLink }),

    beginInteraction: () => set({ interacting: true, _pending: snapshot() }),
    endInteraction: () => set({ interacting: false, _pending: null }),

    setAlgorithm: (i) => commit((d) => { d.patch.algorithm = i }),
    setFeedback: (v) => commit((d) => { d.patch.feedback = clampInt(v, 0, 7) }),
    setOpField: (opIdx, field, value) => commit((d) => { (d.patch.ops[opIdx][field] as string) = value }),
    setOpEnvelope: (opIdx, ep) => commit((d) => { d.patch.ops[opIdx].ep = ep }),
    setLfoKnob: (idx, val) => commit((d) => { d.patch.lfo.knobs[idx].val = clampInt(val, 0, 99) }),
    setFxKnob: (b, k, val) => commit((d) => { d.fx[b].knobs[k].val = clampInt(val, 0, 99) }),
    toggleFxBlock: (b) => commit((d) => { d.fx[b].enabled = !d.fx[b].enabled }),
    setSpxProgram: (i) => commit((d) => { d.spx.program = i }),
    setSpxParam: (i, val) => commit((d) => { d.spx.params[i].val = clampInt(val, 0, 99) }),
    setGenField: (field, value) => commit((d) => { d.gen[field] = value }),
    toggleKeep: (i) => commit((d) => { d.gen.kept[i] = !d.gen.kept[i] }),
    toggleDrumMute: (i) => commit((d) => { d.drums[i].mute = !d.drums[i].mute }),
    toggleDrumSolo: (i) => commit((d) => { d.drums[i].solo = !d.drums[i].solo }),
    setDrumLevel: (i, val) => commit((d) => { d.drums[i].lvl = clampInt(val, 0, 99) }),
    setMidiField: (field, value) => commit((d) => { d.midi[field] = value }),
    startLearn: (rowIdx) => commit((d) => { d.midi.learningRow = rowIdx; d.midi.learnMode = true }),
    clearCc: (rowIdx) => commit((d) => { d.midi.ccRows[rowIdx].cc = '—'; d.midi.ccRows[rowIdx].assigned = false }),

    undo: () => {
      const past = get().past
      if (!past.length) return
      const prev = past[past.length - 1]
      set({ ...clone(prev), past: past.slice(0, -1) })
    },
    canUndo: () => get().past.length > 0,

    audioStub: (action) => {
      // TODO: wire to a real audio engine + MIDI/sysex I/O in a later pass.
      // eslint-disable-next-line no-console
      console.info('[OP4 audio stub]', action)
    },
  }
})

function clampInt(v: number, lo: number, hi: number): number {
  return Math.max(lo, Math.min(hi, Math.round(v)))
}
