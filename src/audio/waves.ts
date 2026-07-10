import type { SynthMode } from '../state/types'

/**
 * Map an operator's wave label to a Web Audio oscillator type.
 * DX21 is a sine-only machine; the TX81Z added selectable operator waves.
 * So in DX21 mode every operator is a sine (authentic), and only in TX81Z
 * mode do the W2/W3/… selections take effect — making the mode toggle audible.
 */
export function waveType(label: string, mode: SynthMode): OscillatorType {
  if (mode === 'DX21') return 'sine'
  const l = label.toUpperCase()
  if (l.includes('SQR') || l.includes('SQ')) return 'square'
  if (l.includes('TRI')) return 'triangle'
  if (l.includes('SAW')) return 'sawtooth'
  return 'sine'
}

/** MIDI note number → frequency (A4 = 69 = 440 Hz). */
export function mtof(midi: number): number {
  return 440 * Math.pow(2, (midi - 69) / 12)
}

/** Parse a ratio label like "14.00" → 14. Falls back to 1. */
export function parseRatio(label: string): number {
  const v = parseFloat(label)
  return Number.isFinite(v) && v > 0 ? v : 1
}

/** Parse a detune label like "+2" / "−4" (unicode minus) → cents. */
export function parseDetuneCents(label: string): number {
  const n = parseInt(label.replace(/[−–]/g, '-'), 10)
  return Number.isFinite(n) ? n * 3 : 0 // ~3 cents per DX detune step
}
