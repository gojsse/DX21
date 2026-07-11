// WebView <-> plugin bridge (additive, no-op in the browser).
//
// When the UI runs inside the JUCE plugin, `window.__JUCE__` is injected. This
// module connects the Zustand store to the native event bus, exchanging the
// UI's *display* patch model (all the mapping to/from the DSP model lives in
// native C++, src/bridge/WebPatch):
//   - outbound: store patch changes  -> emit "op4_webPatch"
//   - inbound:  "op4_initPatch" (initial data) + "op4_setPatch" (runtime push)
//               -> deep-merge into the store
//
// NOTE: the JS<->native round-trip can only be verified in a real host (DAW).
// The native mapping is unit-tested (op4_webpatch_tests); this pipe is not.

import { useStore } from '../state/store'
import type { Patch } from '../state/types'

interface JuceBackend {
  emitEvent: (eventId: string, object: unknown) => void
  addEventListener: (eventId: string, fn: (object: unknown) => void) => void
}
interface JuceGlobal {
  backend?: JuceBackend
  initialisationData?: Record<string, unknown>
}
declare global {
  interface Window { __JUCE__?: JuceGlobal }
}

// True when the UI is embedded in the JUCE plugin (vs. a plain browser). Used to
// disable the web-prototype's own Web Audio "sketch" engine + computer-keyboard
// play so the plugin's sound comes only from the host's MIDI -> the real engine.
export function isInPlugin(): boolean {
  return typeof window !== 'undefined' && !!window.__JUCE__?.backend
}

// Ask the plugin to perform a native action. Each returns false in the browser
// so callers can fall back to their web behaviour.
function emit(event: string, obj: unknown = {}): boolean {
  const backend = window.__JUCE__?.backend
  if (!backend) return false
  backend.emitEvent(event, obj)
  return true
}
export const requestLoadSyx = (): boolean => emit('op4_loadSyx')      // open a .syx file
export const requestExportSyx = (): boolean => emit('op4_exportSyx')  // save current voice
export const requestSendVoice = (): boolean => emit('op4_sendVoice')  // transmit over MIDI
export const requestSelectVoice = (index: number): boolean => emit('op4_selectVoice', { index })

let applyingInbound = false

// A partial display patch from native (native-owned fields only).
type PartialPatch = Partial<Patch> & { ops?: Partial<Patch['ops'][number]>[] }

function parse(raw: unknown): PartialPatch {
  if (typeof raw === 'string') { try { return JSON.parse(raw) } catch { return {} } }
  return (raw as PartialPatch) ?? {}
}

// Deep-merge a partial display patch into the store, leaving UI-only fields
// (per-op name/role, sizzler, ...) untouched.
function applyInbound(raw: unknown): void {
  const w = parse(raw)
  applyingInbound = true
  try {
    useStore.setState((s) => {
      const patch: Patch = { ...s.patch }
      if (typeof w.algorithm === 'number') patch.algorithm = w.algorithm
      if (typeof w.feedback === 'number') patch.feedback = w.feedback
      if (Array.isArray(w.ops)) {
        patch.ops = patch.ops.map((op, i) => {
          const nw = w.ops![i]
          if (!nw) return op
          return { ...op, ...nw, ep: { ...op.ep, ...(nw.ep ?? {}) } }
        })
      }
      if (w.lfo) {
        patch.lfo = { ...patch.lfo }
        if (typeof w.lfo.wave === 'string') patch.lfo.wave = w.lfo.wave
        if (typeof w.lfo.sync === 'boolean') patch.lfo.sync = w.lfo.sync
        if (Array.isArray(w.lfo.knobs)) {
          patch.lfo.knobs = patch.lfo.knobs.map((k) => {
            const nk = w.lfo!.knobs!.find((x) => x.key === k.key)
            return nk ? { ...k, val: nk.val } : k
          })
        }
      }
      if (w.fn) patch.fn = { ...patch.fn, ...w.fn }
      return { patch }
    })
  } finally {
    applyingInbound = false
  }
}

export function connectPluginBridge(): void {
  const juce = window.__JUCE__
  if (!juce?.backend) return // running in the browser — nothing to do
  const backend = juce.backend

  // inbound: initial patch + runtime pushes (host state recall / automation)
  const initial = juce.initialisationData?.op4_initPatch
  if (initial !== undefined) applyInbound(initial)
  backend.addEventListener('op4_setPatch', applyInbound)

  // inbound: loaded VMEM bank -> Library grid
  backend.addEventListener('op4_bank', (info) => {
    const b = (typeof info === 'string' ? JSON.parse(info) : info) as
      { loaded?: boolean; current?: number; names?: string[] }
    useStore.getState().setBank({
      loaded: !!b?.loaded, current: b?.current ?? 0, names: b?.names ?? [],
    })
  })

  // outbound: forward the whole display patch on any change; native maps it.
  useStore.subscribe((s, prev) => {
    if (applyingInbound) return
    if (s.patch === prev.patch) return // Zustand replaces patch by value on edit
    backend.emitEvent('op4_webPatch', s.patch)
  })
}
