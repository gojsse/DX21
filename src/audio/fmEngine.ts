import type { Patch, SynthMode } from '../state/types'
import { ALGS } from '../state/seed'
import { waveType, mtof, parseRatio, parseDetuneCents } from './waves'

// Peak modulation index (a level-99 modulator deviates the carrier by
// INDEX × its own frequency). Tuned for a rich-but-not-harsh 4-op range.
const INDEX = 5
const FB_SCALE = 2.2 // OP4 self-feedback depth at feedback=7
const VOICE_GAIN = 0.5

/** Envelope timings (seconds) derived from the normalized {a,d1,r} stages. */
function envTimes(a: number, d1: number, r: number) {
  return { atk: 0.003 + a * 2.0, dec: 0.02 + d1 * 1.4, rel: 0.02 + r * 2.0 }
}

interface OpNodes {
  osc: OscillatorNode
  env: GainNode // ADSR, 0..1
  scale: GainNode // role-dependent constant (carrier level, or modulator index in Hz)
  freq: number
  rel: number // release time (s), cached for noteOff
}

/** A single sounding note: builds the operator graph for the current patch. */
export class Voice {
  private ctx: BaseAudioContext
  private ops: OpNodes[] = []
  private voiceGain: GainNode
  private lfo?: OscillatorNode
  private stopTime = Infinity
  readonly startedAt: number
  readonly note: number

  constructor(ctx: BaseAudioContext, dest: AudioNode, patch: Patch, mode: SynthMode, note: number, when: number) {
    this.ctx = ctx
    this.note = note
    this.startedAt = when
    const base = mtof(note)
    const alg = ALGS[patch.algorithm]
    const carriers = new Set(alg.carriers)

    this.voiceGain = ctx.createGain()
    this.voiceGain.gain.value = VOICE_GAIN / Math.sqrt(alg.carriers.length)
    this.voiceGain.connect(dest)

    // Build the four operators.
    for (let i = 0; i < 4; i++) {
      const op = patch.ops[i]
      const isCarrier = carriers.has(i + 1)
      const freq = base * parseRatio(op.ratio)
      const level = clamp01(parseInt(op.lvl, 10) / 99)
      const { atk, dec, rel } = envTimes(op.ep.a, op.ep.d1, op.ep.r)

      const osc = ctx.createOscillator()
      osc.type = waveType(op.wave, mode)
      osc.frequency.value = freq
      osc.detune.value = parseDetuneCents(op.det)

      const env = ctx.createGain()
      const scale = ctx.createGain()
      scale.gain.value = isCarrier ? level : level * INDEX * freq
      osc.connect(env).connect(scale)

      // ADSR on the env gain (0 → 1 → sustain).
      env.gain.setValueAtTime(0.0001, when)
      env.gain.linearRampToValueAtTime(1, when + atk)
      env.gain.linearRampToValueAtTime(Math.max(0.0001, op.ep.s), when + atk + dec)

      this.ops.push({ osc, env, scale, freq, rel })
    }

    // Routing: modulator scale → carrier oscillator frequency.
    for (const [mod, car] of alg.edges) {
      this.ops[mod - 1].scale.connect(this.ops[car - 1].osc.frequency)
    }
    // Carriers → voice output.
    for (const c of alg.carriers) this.ops[c - 1].scale.connect(this.voiceGain)

    // OP4 feedback: self-FM through a one-quantum delay (the only legal way to
    // form an audio cycle in Web Audio). Bounded by a conservative scale.
    if (patch.feedback > 0) {
      const op4 = this.ops[3]
      const fb = ctx.createGain()
      fb.gain.value = (patch.feedback / 7) * FB_SCALE * op4.freq
      const delay = ctx.createDelay(0.05)
      delay.delayTime.value = 128 / ctx.sampleRate
      op4.env.connect(fb).connect(delay).connect(op4.osc.frequency)
    }

    // LFO → pitch of every operator (PMD). Amp-mod (AMD) kept out of v1.
    const speed = patch.lfo.knobs.find((k) => k.key === 'speed')?.val ?? 0
    const pmd = patch.lfo.knobs.find((k) => k.key === 'pmd')?.val ?? 0
    if (speed > 0 && pmd > 0) {
      const lfo = ctx.createOscillator()
      lfo.type = lfoWave(patch.lfo.wave)
      lfo.frequency.value = 0.1 + (speed / 99) * 12 // ~0.1–12 Hz
      const lg = ctx.createGain()
      lg.gain.value = (pmd / 99) * 50 // cents of pitch wobble
      lfo.connect(lg)
      for (const o of this.ops) lg.connect(o.osc.detune)
      lfo.start(when)
      this.lfo = lfo
    }

    for (const o of this.ops) o.osc.start(when)
  }

  /** Release the note; oscillators stop after the longest release tail. */
  noteOff(when: number) {
    let maxRel = 0.05
    for (const op of this.ops) {
      maxRel = Math.max(maxRel, op.rel)
      const g = op.env.gain
      g.cancelScheduledValues(when)
      g.setValueAtTime(Math.max(0.0001, g.value), when)
      g.linearRampToValueAtTime(0.0001, when + op.rel)
    }
    this.stopTime = when + maxRel + 0.05
    for (const o of this.ops) o.osc.stop(this.stopTime)
    this.lfo?.stop(this.stopTime)
  }

  /** Live-update level/ratio/detune/wave of a sustaining voice (knob feedback). */
  updateLive(patch: Patch, mode: SynthMode) {
    const t = this.ctx.currentTime
    const base = mtof(this.note)
    const carriers = new Set(ALGS[patch.algorithm].carriers)
    for (let i = 0; i < 4; i++) {
      const op = patch.ops[i]
      const node = this.ops[i]
      const freq = base * parseRatio(op.ratio)
      const level = clamp01(parseInt(op.lvl, 10) / 99)
      node.osc.frequency.setTargetAtTime(freq, t, 0.02)
      node.osc.detune.setTargetAtTime(parseDetuneCents(op.det), t, 0.02)
      node.osc.type = waveType(op.wave, mode)
      node.scale.gain.setTargetAtTime(carriers.has(i + 1) ? level : level * INDEX * freq, t, 0.02)
    }
  }

  get endsAt() { return this.stopTime }
}

function lfoWave(w: string): OscillatorType {
  const l = w.toUpperCase()
  if (l.includes('SQ')) return 'square'
  if (l.includes('SAW')) return 'sawtooth'
  if (l.includes('SIN')) return 'sine'
  return 'triangle'
}
function clamp01(v: number) { return Math.max(0, Math.min(1, Number.isFinite(v) ? v : 0)) }

/** Polyphonic engine: owns the AudioContext, master bus, and voice pool. */
export class FMEngine {
  ctx: AudioContext
  master: GainNode
  private comp: DynamicsCompressorNode
  private voices = new Map<number, Voice>()
  private maxVoices = 8

  constructor() {
    this.ctx = new AudioContext()
    this.master = this.ctx.createGain()
    this.master.gain.value = 0.85
    this.comp = this.ctx.createDynamicsCompressor()
    this.master.connect(this.comp).connect(this.ctx.destination)
  }

  get running() { return this.ctx.state === 'running' }
  resume() { return this.ctx.resume() }

  noteOn(note: number, patch: Patch, mode: SynthMode) {
    if (this.ctx.state !== 'running') void this.ctx.resume()
    const existing = this.voices.get(note)
    if (existing) existing.noteOff(this.ctx.currentTime) // retrigger
    if (this.voices.size >= this.maxVoices) this.stealOldest()
    this.voices.set(note, new Voice(this.ctx, this.master, patch, mode, note, this.ctx.currentTime))
  }

  noteOff(note: number) {
    const v = this.voices.get(note)
    if (!v) return
    v.noteOff(this.ctx.currentTime)
    this.voices.delete(note)
  }

  /** Push live patch edits into all sustaining voices. */
  updateLive(patch: Patch, mode: SynthMode) {
    for (const v of this.voices.values()) v.updateLive(patch, mode)
  }

  private stealOldest() {
    let oldest: Voice | undefined
    for (const v of this.voices.values()) if (!oldest || v.startedAt < oldest.startedAt) oldest = v
    if (oldest) this.noteOff(oldest.note)
  }
}

/**
 * Render a single sustained note through an OfflineAudioContext — used for
 * headless verification (confirm the patch is audible and that edits change it).
 */
export async function renderPatchOffline(patch: Patch, mode: SynthMode, note: number, seconds: number): Promise<Float32Array> {
  const sr = 44100
  const oac = new OfflineAudioContext(1, Math.ceil(seconds * sr), sr)
  const master = oac.createGain()
  master.gain.value = 0.85
  master.connect(oac.destination)
  // eslint-disable-next-line no-new
  new Voice(oac, master, patch, mode, note, 0)
  const buf = await oac.startRendering()
  return buf.getChannelData(0)
}
