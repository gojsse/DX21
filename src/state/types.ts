import type { ThemeName } from '../theme/themes'

export type ViewId = 'voice' | 'lib' | 'drums' | 'fx' | 'gen' | 'midi'
export type SynthMode = 'DX21' | 'TX81Z'

/** ADSR envelope, normalized 0..1 per stage (matches the handoff's env() input). */
export interface Envelope {
  a: number
  d1: number
  s: number
  r: number
}

export interface Operator {
  name: string
  role: 'CARRIER' | 'MOD'
  carrier: boolean
  ratio: string
  lvl: string
  det: string
  wave: string
  fix: boolean
  eg: string
  ep: Envelope
}

export interface LfoKnob {
  key: 'speed' | 'delay' | 'pmd' | 'amd'
  label: string
  val: number
}

export interface Patch {
  bank: string
  slot: string
  name: string
  edited: boolean
  algorithm: number // 0..7
  feedback: number // 0..99 style, displayed as small int here
  ops: Operator[]
  lfo: { wave: string; sync: boolean; knobs: LfoKnob[] }
  pitchEg: { label: string }
  fn: { voice: 'POLY' | 'MONO'; porta: number; transpose: number; assign: 'SNGL' | 'DUAL' | 'SPLIT' }
  sizzler: { amount: number; mode: 'RND' | 'MRF' | 'DRF'; targets: number; freeze: boolean }
}

export interface UiState {
  view: ViewId
  mode: SynthMode
  theme: ThemeName
  hardwareLink: boolean
}
