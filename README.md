# OP4 — FM Synth (Web UI + C++ Plugin)

**A complete software instrument** — plugin + standalone — recreating Yamaha's 1985–87 4-operator FM synths (**DX21 / TX81Z**), plus an RX21-style drum engine, vintage FX, a sysex patch librarian, and a patch-bank generator.

The design goal: **make 4-op FM legible on one screen.** The moat: **edit and librarian real hardware over MIDI** — many FM plugins exist; few are also first-class editors for the actual boxes.

See [`docs/plans/`](./docs/plans/) for the design brief, master plan, and technical spec.

---

## What's here

### `/src` — React web prototype (production UI)
- **6 views:** Voice, Library, Drums, FX, Generator, MIDI/Settings (all pixel-faithful to the design handoff)
- **3 themes:** DX21 charcoal/green, TX81Z black/orange, SK-1 cream/teal — runtime-switchable CSS tokens
- **Live controls:** draggable knobs, ADSR envelopes, editable value chips, sliders, toggles, segmented switches
- **Web Audio FM engine:** 4-op FM with verified algorithm topologies, undo/redo with gesture coalescing, keyboard play (A–K)
- **Tech:** Vite + React + TypeScript + Zustand

**Status:** feature-complete, verified against screenshots. This UI will be embedded in the JUCE plugin (M1+).

### `src/` (C++) — JUCE plugin scaffold (M0)
Empty plugin structure with stubs for:
- **`PluginProcessor`** — AudioProcessor with APVTS parameter tree (to come)
- **`FMEngine`** — ymfm-based 4-op voice engine skeleton (M1)
- **Sysex codecs** — VCED/ACED/VMEM header definitions (M2)
- **MIDI** — NRPN/CC table placeholders (M4)

**Status:** skeleton. CMakeLists.txt targets VST3 / AU / CLAP / Standalone.

### `docs/plans/` — Complete planning
- **`op4-design-brief.md`** — vision, aesthetic, the six views, interaction principles
- **`fm-synth-plugin-plan.md`** — master plan: scope, roadmap (M0–M6), risk, acceptance criteria
- **`op4-technical-handoff.md`** — data formats, byte tables, sysex specs, algorithm tables, decision log
- **`design_handoff_op4_synth/`** — high-fidelity design reference (UI screenshots + `.dc.html` interactive prototype)

---

## Run the web prototype

```bash
npm install
npm run dev       # dev server, live reload
npm run build     # typecheck + production build (64 kB gzipped)
npm run preview   # preview the production build
```

Open http://localhost:5174 → click nav to cycle views, `THEME` to switch palettes, `A–K` to play.

## Build the C++ plugin (M0)

```bash
git submodule update --init --recursive  # ymfm
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
# Outputs: VST3, AU, CLAP, Standalone in build/OP4_artefacts/
```

**Status:** empty plugin passes CMake. MIDI/sysex/DSP plumbing is M1+ work.

---

## Project phases (see plan §7)

| Phase | What | Status |
|-------|------|--------|
| **M0** | Scaffold (JUCE + CI) | in-progress |
| **M1** | Voice engine + UI (spike WebView) | pending |
| **M2** | Sysex I/O + Librarian | pending → **ships as Alpha** |
| **M3** | FX + Drums | pending |
| **M4** | MIDI depth (NRPN/CC/Sizzler/tuning) | pending |
| **M5** | Generator | pending |
| **M6** | Hardening + release | pending → **ships as 1.0** |

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
