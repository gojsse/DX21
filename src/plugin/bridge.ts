// WebView <-> plugin bridge (additive, no-op in the browser).
//
// When the UI runs inside the JUCE plugin, `window.__JUCE__` is injected. This
// module connects the Zustand store to the native event bus:
//   - outbound: store patch changes  -> emit "op4_applyPatch" (canonical JSON)
//   - inbound:  native "op4_patch" / initial data -> apply to the store
//
// The wire format is the *canonical native Patch* (see src/bridge/PatchJson.*).
// This first pass maps the unambiguous numeric fields (algorithm, feedback);
// the operator / LFO / function fields are display strings in the UI model and
// their mapping is a follow-up that pairs with the DSP calibration work.
//
// NOTE: the JS<->native round-trip can only be verified in a real host (DAW).
// The native half is unit-tested (op4_patchjson_tests); this half is not.

import { useStore } from '../state/store'

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

/** Canonical (native) patch subset we currently bridge. */
interface CanonicalPatch {
  algorithm?: number
  feedback?: number
  // TODO(M1): operators[], lfo, pitchEg, fn — needs display<->DSP mapping.
}

let applyingInbound = false

function applyCanonical(raw: unknown): void {
  const c: CanonicalPatch =
    typeof raw === 'string' ? safeParse(raw) : (raw as CanonicalPatch) ?? {}

  applyingInbound = true
  try {
    useStore.setState((s) => {
      const patch = { ...s.patch }
      if (typeof c.algorithm === 'number') patch.algorithm = c.algorithm
      if (typeof c.feedback === 'number') patch.feedback = c.feedback
      return { patch }
    })
  } finally {
    applyingInbound = false
  }
}

function safeParse(s: string): CanonicalPatch {
  try {
    return JSON.parse(s) as CanonicalPatch
  } catch {
    return {}
  }
}

export function connectPluginBridge(): void {
  const juce = window.__JUCE__
  if (!juce?.backend) return // running in the browser — nothing to do

  const backend = juce.backend

  // inbound: initial patch delivered via withInitialisationData("op4_patch", ...)
  const initial = juce.initialisationData?.op4_patch
  if (initial !== undefined) applyCanonical(initial)

  // inbound: runtime pushes (host automation / state recall)
  backend.addEventListener('op4_patch', applyCanonical)

  // outbound: forward store patch changes as a partial canonical patch. The
  // native side merges, so sending only changed fields is safe.
  let last = { algorithm: -1, feedback: -1 }
  useStore.subscribe((s) => {
    if (applyingInbound) return
    const { algorithm, feedback } = s.patch
    if (algorithm === last.algorithm && feedback === last.feedback) return
    last = { algorithm, feedback }
    backend.emitEvent('op4_applyPatch', { algorithm, feedback })
  })
}
