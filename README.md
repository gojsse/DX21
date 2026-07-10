# OP4 — 4-Operator FM Synth UI

An interactive recreation of the **OP4** FM synth plugin interface — a modern take on
Yamaha's 1985–87 4-operator FM synths (DX21 / TX81Z), an RX21-style drum engine, a
vintage effects section, a sysex patch librarian, and a themed patch-bank generator.
The design goal: **make deep 4-op FM programming legible on one screen.**

Built from the design handoff in `docs/plans/design_handoff_op4_synth/`.

## Stack

- **Vite + React + TypeScript**
- **Zustand** for state (patch model, per-view state, undo stack)
- Theme system as **CSS custom properties** — three exact themes (DX21 / TX81Z / SK-1),
  runtime-switchable, restyling every view at once
- All UI is CSS/SVG — knobs, algorithm glyphs, and envelope graphs are drawn, no bitmaps

## Run

```bash
npm install
npm run dev      # http://localhost:5173
npm run build    # typecheck + production build
```

## What's interactive

- **Nav** — six views (Voice · Library · Drums · FX · Generator · MIDI/Settings)
- **Theme** — click the footer `THEME …▸` to cycle DX21 → TX81Z → SK-1
- **Mode** — DX21 ⇄ TX81Z toggle on Voice reveals/dims the TX-only controls
- **Live controls** — draggable knobs and ADSR envelope nodes, editable value chips
  (double-click), sliders, toggles, segmented switches
- **Sound** — a real 4-operator FM engine on the Web Audio API. Play with the
  **A–K** keys (Z/X shift octave); clicking a patch in Library or the ‹/› patch
  nav auditions it. Knob/envelope edits update sustaining notes live.
- **Undo everywhere** — Cmd/Ctrl+Z; one drag gesture = one undo entry
- **Hardware link** — click the header link indicator to toggle

## Structure

```
src/
  App.tsx            chassis + header/nav + footer + view router
  theme/             themes.ts (token maps), tokens.ts (shared styles)
  state/             store.ts (Zustand + undo), seed.ts, types.ts
  audio/             fmEngine.ts (4-op FM), waves.ts, useAudio.ts (keyboard + live)
  components/        Knob, EnvelopeGraph, AlgorithmGlyph, Slider, Toggle, Editable
  views/             one component per view
```

## FM engine notes

`audio/fmEngine.ts` builds a native Web Audio graph per note: each operator is an
`OscillatorNode → env (ADSR) → scale`, with modulator `scale` feeding the target
carrier's `frequency` AudioParam (true FM). The algorithm's edges/carriers drive the
routing, OP4 gets self-feedback via a one-quantum `DelayNode` loop, and a per-voice LFO
adds pitch mod (PMD). 8-voice polyphony with oldest-note stealing; a master compressor
guards against clipping. DX21 forces sine operators; TX81Z honors per-op waves.
Still approximate vs. real hardware (no pitch-EG, amp-LFO, or fixed-frequency mode yet).

## Not yet wired (intentional — see docs handoff)

- **Audio + MIDI/sysex I/O** are stubbed (`store.audioStub`) — this pass is the UI layer.
- The right-click "assign CC / add to Sizzler" context menu is reserved but not drawn.

The 8 algorithm topologies in `state/seed.ts` have been **corrected and verified** against
the real Yamaha 4-op chart (DX21/DX27/DX100/TX81Z) using the TX81Z connection matrix in
[gesellkammer/sc-tx81z](https://github.com/gesellkammer/sc-tx81z).
