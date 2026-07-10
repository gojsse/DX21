import { useEffect, useState } from 'react'
import { FMEngine } from './fmEngine'
import { useStore } from '../state/store'

// Lazily-created singleton — one AudioContext for the whole app.
let engine: FMEngine | null = null
export function getEngine(): FMEngine {
  if (!engine) engine = new FMEngine()
  return engine
}

// QWERTY → semitone offset from the current base octave (one octave + a bit).
const KEYMAP: Record<string, number> = {
  a: 0, w: 1, s: 2, e: 3, d: 4, f: 5, t: 6, g: 7, y: 8, h: 9, u: 10, j: 11,
  k: 12, o: 13, l: 14, p: 15,
}

/**
 * Installs computer-keyboard playing (A–K row, Z/X shift octave), resumes the
 * AudioContext on first gesture, and streams live patch edits into any
 * sustaining voices. Returns whether audio is running (for the UI indicator).
 */
export function useAudio() {
  const [running, setRunning] = useState(false)

  useEffect(() => {
    const eng = getEngine()
    let octave = 0
    const held = new Set<string>()

    const isEditingField = () => {
      const el = document.activeElement
      return !!el && (el.tagName === 'INPUT' || el.tagName === 'TEXTAREA')
    }

    const onKeyDown = (e: KeyboardEvent) => {
      if (e.metaKey || e.ctrlKey || e.altKey || e.repeat || isEditingField()) return
      const key = e.key.toLowerCase()
      if (key === 'z') { octave = Math.max(-3, octave - 1); return }
      if (key === 'x') { octave = Math.min(3, octave + 1); return }
      const off = KEYMAP[key]
      if (off === undefined || held.has(key)) return
      held.add(key)
      void eng.resume().then(() => setRunning(eng.running))
      const { patch, mode } = useStore.getState()
      eng.noteOn(60 + octave * 12 + off, patch, mode)
    }

    const onKeyUp = (e: KeyboardEvent) => {
      const key = e.key.toLowerCase()
      const off = KEYMAP[key]
      if (off === undefined || !held.has(key)) return
      held.delete(key)
      eng.noteOff(60 + octave * 12 + off)
    }

    window.addEventListener('keydown', onKeyDown)
    window.addEventListener('keyup', onKeyUp)

    // Stream live patch/mode edits to sustaining voices.
    const unsub = useStore.subscribe((s, prev) => {
      if (s.patch !== prev.patch || s.mode !== prev.mode) eng.updateLive(s.patch, s.mode)
    })

    const poll = window.setInterval(() => setRunning(eng.running), 500)

    return () => {
      window.removeEventListener('keydown', onKeyDown)
      window.removeEventListener('keyup', onKeyUp)
      unsub()
      window.clearInterval(poll)
    }
  }, [])

  return { running }
}

/** One-shot audition (e.g. clicking a patch) — plays a short middle-C note. */
export function auditionNote(note = 60, ms = 700) {
  const eng = getEngine()
  const { patch, mode } = useStore.getState()
  void eng.resume()
  eng.noteOn(note, patch, mode)
  window.setTimeout(() => eng.noteOff(note), ms)
}
