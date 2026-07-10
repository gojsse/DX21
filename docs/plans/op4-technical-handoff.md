# OP4 — Technical Handoff Spec (for Claude Code)

Companion to `fm-synth-plugin-plan.md`. This document is the implementation reference: data formats, tables, and parameter models. Where a value is marked **[verify]**, confirm against a real hardware dump or the Yamaha service manual before locking the round-trip tests — the structures below are correct in shape, but byte-level details must be validated with fixtures, not trusted from memory.

---

## 0. Status, prototype reconciliation & open architecture decisions (added 2026-07)

**What already exists.** A working **web prototype** lives in this repo (`/src`, Vite + React + TypeScript): all six views at full fidelity, the three-theme token system, live vector controls, undo, and a **Web Audio 4-operator FM engine** (`src/audio/fmEngine.ts`) playable from the computer keyboard. It is not the shipping plugin — but it is a validated, interactive reference for the UI *and* the voice architecture, and it changes several assumptions below. Treat it as the source of truth for layout/interaction and as a DSP sketch to null the C++ engine against.

**The companion `fm-synth-plugin-plan.md` now exists** (`docs/plans/fm-synth-plugin-plan.md`, written 2026-07-09) and is the **authoritative** source for vision, scope, versioning, roadmap, risk, and acceptance. Section 17 below is a condensed mirror of its roadmap kept here for convenience — if the two ever disagree, the plan wins.

**Pivotal decision not yet made — the UI layer.** This handoff's scaffold (§1) assumes a native JUCE `Component`/`LookAndFeel` UI. But we now have a complete React UI. Two viable paths:

- **A — WebView-hosted (reuse the prototype).** Ship the existing React UI inside the plugin via a webview (JUCE 8 `WebBrowserComponent` with the native↔JS bridge, or `choc::ui::WebView`). Pros: the UI is *done*, re-themes for free, one codebase for standalone-web demo + plugin. Cons: webview↔DSP parameter bridge, per-DAW webview quirks, larger binary, some latency on high-rate meters (envelope/op-pulse animation must run in JS from pushed values).
- **B — Native re-implementation.** Rebuild the six views in JUCE FlexBox/Grid with the token set as a `LookAndFeel` palette. Pros: tightest integration, best automation/accessibility, smallest binary. Cons: re-doing finished work; keeping two UIs in sync if the web demo stays.

Recommendation: **prototype path A first** (fastest to an audible, playable plugin that looks right), keep B as the fallback if webview integration or automation fidelity disappoints. Record the decision here once made. This choice cascades into §9 (threading), §10 (state), and §15 (binary size / CI).

**Role of the Web Audio engine.** It is a *musical approximation*, not the production DSP. Production DSP is **ymfm** (§3, §8) for hardware-accurate output and round-trippable parameter behavior. Keep the Web Audio engine for: the browser demo, quick UI-side auditioning if path A is chosen (optional), and as a cheap "does this patch make plausible sound" oracle in generator scoring during prototyping. Do **not** let it drift into being the shipping voice.

---

## 1. Repo Scaffold (Phase 0)

```
op4/
├── CMakeLists.txt            # JUCE 8 via FetchContent; targets: VST3, AU, CLAP, Standalone
├── libs/ymfm/                # git submodule (BSD-3) — OPM/OPZ cores
├── src/
│   ├── engine/               # FMEngine, Voice, Operator, Algorithms, LFO, Envelope
│   ├── model/                # Patch (neutral model), DrumKit, enums, ranges
│   ├── sysex/                # VCEDCodec, ACEDCodec, VMEMCodec, SysexRouter
│   ├── fx/                   # FX500Chain, SPX90Block, dsp/
│   ├── midi/                 # MEP4Echo, CCTable, NRPNCodec, Sizzler
│   ├── generator/            # Templates, Mutator, Scorer, Namer, BankWriter
│   ├── librarian/            # Library DB (SQLite or JSON), BankModel
│   └── ui/                   # per design brief
├── tests/                    # Catch2; fixtures/ holds real .syx dumps
└── resources/                # factory banks, microtuning tables
```

Neutral `Patch` model is the hub. VCED/ACED/VMEM are codecs to/from it; the engine, generator, librarian, and UI never touch raw sysex.

---

## 2. Sysex Formats

All Yamaha 4-op family. `n` = device number (0–15). Checksum = two's complement of the sum of data bytes, masked to 7 bits: `(128 - (sum & 0x7F)) & 0x7F`.

### 2.1 VCED — Single Voice (DX21/DX27/DX100 native; TX81Z base layer)

```
F0 43 0n 03 00 5D <93 data bytes> <checksum> F7
```

Byte count 0x005D = 93. Operator data is transmitted in order **OP4, OP2, OP3, OP1** (13 bytes each, offsets 0–51) **[verify order against a fixture]**, then common parameters.

Per-operator block (13 bytes):

| Off | Parameter | Range |
|----|------------|-------|
| +0 | Attack Rate | 0–31 |
| +1 | Decay 1 Rate | 0–31 |
| +2 | Decay 2 Rate | 0–31 |
| +3 | Release Rate | 1–15 |
| +4 | Decay 1 Level | 0–15 |
| +5 | Level Scaling | 0–99 |
| +6 | Rate Scaling | 0–3 |
| +7 | EG Bias Sensitivity | 0–7 |
| +8 | AM Enable | 0–1 |
| +9 | Key Velocity Sensitivity | 0–7 |
| +10 | Output Level | 0–99 |
| +11 | Frequency (coarse ratio index) | 0–63 |
| +12 | Detune | 0–6 (3 = center) |

Common block (offsets 52–92):

| Off | Parameter | Range |
|-----|-----------|-------|
| 52 | Algorithm | 0–7 |
| 53 | Feedback (OP4) | 0–7 |
| 54 | LFO Speed | 0–99 |
| 55 | LFO Delay | 0–99 |
| 56 | Pitch Mod Depth | 0–99 |
| 57 | Amp Mod Depth | 0–99 |
| 58 | LFO Sync | 0–1 |
| 59 | LFO Wave (saw/sqr/tri/S&H) | 0–3 |
| 60 | Pitch Mod Sensitivity | 0–7 |
| 61 | Amp Mod Sensitivity | 0–3 |
| 62 | Transpose | 0–48 (24 = C3) |
| 63 | Poly/Mono | 0–1 |
| 64 | Pitch Bend Range | 0–12 |
| 65 | Portamento Mode | 0–1 |
| 66 | Portamento Time | 0–99 |
| 67 | Foot Control Volume | 0–99 |
| 68 | Sustain switch | 0–1 |
| 69 | Portamento switch | 0–1 |
| 70 | Chorus switch | 0–1 |
| 71 | Mod Wheel → Pitch | 0–99 |
| 72 | Mod Wheel → Amplitude | 0–99 |
| 73 | Breath → Pitch | 0–99 |
| 74 | Breath → Amplitude | 0–99 |
| 75 | Breath → Pitch Bias | 0–99 (50 = center) |
| 76 | Breath → EG Bias | 0–99 |
| 77–86 | Voice Name | 10 × ASCII |
| 87–89 | Pitch EG Rates 1–3 (DX21/DX11; TX81Z ignores) | 0–99 |
| 90–92 | Pitch EG Levels 1–3 | 0–99 |

### 2.2 ACED — TX81Z Additional Voice Data

```
F0 43 0n 7E 00 21 "LM  8976AE" <23 data bytes> <checksum> F7
```

Format 0x7E (universal bulk) with 10-char ASCII classification `LM  8976AE`; byte count 0x0021 = 33 covers classification + data. Per-op (5 bytes × 4, same OP4/2/3/1 order):
Fixed Frequency on/off (0–1), Fixed Freq Range (0–7), Frequency Range Fine (0–15), Operator Waveform (0–7, W1–W8), EG Shift (0–3). Common: Reverb Rate (0–7), Foot Ctrl → Pitch (0–99), Foot Ctrl → Amplitude (0–99). **[verify field order]**

TX81Z voice transmit sends ACED immediately followed by VCED; the codec must accept both orders and standalone VCED.

### 2.3 VMEM — 32-Voice Packed Bank

```
F0 43 0n 04 20 00 <4096 data bytes> <checksum> F7
```

32 voices × 128 bytes, bit-packed (multiple params share bytes — e.g., detune/rate-scaling packed together). Implement `VMEM ↔ Patch[32]` with a bit-field map derived empirically: take a factory bank dump, unpack, re-pack, assert byte-identical. TX81Z banks also carry PMEM (performance) and system data — parse and preserve as opaque blobs in v1 so re-export doesn't destroy them.

### 2.4 Parser Requirements

Stream-safe (sysex may arrive fragmented over MIDI), tolerant of missing/extra F7, validates checksum but offers "import anyway" on mismatch (many real-world .syx files have bad checksums), auto-detects format from header. Round-trip tests are the acceptance gate: `parse(bytes) → serialize() == bytes` for every fixture.

**Fixture shopping list:** DX21 factory bank .syx, TX81Z factory banks A–D, a handful of community single-voice dumps. All freely available in the usual archives; put them in `tests/fixtures/`.

---

## 3. Algorithm Table

Eight algorithms; these correspond 1:1 to the YM2151/OPM `CONECT` modes 0–7, so **derive the authoritative routing from ymfm's OPM connection code** — treat the table below as the human-readable summary. Feedback is always on OP4. Slot mapping: OPM M1/C1/M2/C2 = OP4/OP3/OP2/OP1.

| Alg | Routing | Carriers |
|----|----------|----------|
| 1 | 4→3→2→1 | 1 |
| 2 | (3+4)→2→1 | 1 |
| 3 | (3→2)+4 → 1 | 1 |
| 4 | (4→3)+2 → 1 | 1 |
| 5 | (2→1) + (4→3) | 1, 3 |
| 6 | 4 → 1, 2, 3 (one modulator, three carriers) | 1, 2, 3 |
| 7 | (4→3) + 2 + 1 | 1, 2, 3 |
| 8 | 1 + 2 + 3 + 4 parallel | 1, 2, 3, 4 |

~~**[verify]** the modulator branch assignment on algs 3 vs 4~~ — **resolved 2026-07.** Decoded from the TX81Z connection matrix in [`gesellkammer/sc-tx81z`](https://github.com/gesellkammer/sc-tx81z) (its Csound `kALG` matrix + the manual's algorithm card) and applied to the prototype: **Alg 3 = `2→1, 4→1, 3→2`** (carrier 1); **Alg 4 = `2→1, 3→1, 4→3`** (carrier 1) — exactly the table above. Still cross-check against ymfm's OPZ `con` wiring when the C++ engine lands (should agree), but the routing is no longer guesswork. Note: the TX81Z is **OPZ (YM2414)**, which shares OPM's 8 CONECT topologies; confirm the DX21/27/100 core shares them too (it does at the topology level) and which ymfm core best models each machine (§8).

Frequency: coarse index 0–63 maps to the DX ratio table (0.50, 0.71, 0.78, 0.87, 1.00, 1.41, 1.57, 1.73, 2.00 … up to ~27.57) — extract the full 64-entry table from the manual and store in `resources/`. TX81Z fine range extends this; fixed-frequency mode uses range+fine to address 8 Hz–~50 kHz **[verify table]**.

---

## 4. MIDI Spec

### 4.1 Performance CCs (defaults, user-remappable)

| CC | Target |
|----|--------|
| 1 | LFO → Pitch depth (mod wheel) |
| 2 | Breath (routes per patch bytes 73–76) |
| 5 | Portamento time |
| 7 | Master volume |
| 10 | Pan |
| 64 | Sustain |
| 65 | Portamento on/off |
| 71 | "Brightness" macro → summed modulator output levels |
| 72 | Release macro → all RR |
| 73 | Attack macro → all AR |
| 74 | "Timbre" macro → feedback |
| 75–79 | Macros 5–8 (user-assigned) |
| 80–83 | Sizzler: amount / rate / target-set / freeze |
| 91 | SPX90 send/mix |
| 93 | FX500 modulation depth |

### 4.2 NRPN — full parameter address space

NRPN MSB = section, LSB = parameter index; value via Data Entry MSB (+LSB for 14-bit where range > 127).

| MSB | Section | LSB indexing |
|-----|---------|--------------|
| 0 | Voice common | VCED common offset − 52 |
| 1–4 | OP1–OP4 | VCED per-op offset 0–12 |
| 5 | ACED extensions | 0–19 per-op, 20–22 common |
| 6 | FX500 | chain param index |
| 7 | SPX90 | algorithm + param index |
| 8 | MEP4 echo | 0=on, 1=repeats, 2=time, 3=transpose, 4=vel decay, 5=sync |
| 9 | Drum rack | slot × 16 + param |

Every NRPN target is also a host-automatable parameter (JUCE `AudioProcessorValueTreeState`); the CC table is a mapping layer on top, and parameter changes echo to MIDI out (togglable) so the plugin doubles as a hardware editor.

### 4.3 MEP4 Echo

Pre-engine MIDI processor. Params: repeats (0–8), delay time (ms or host-sync divisions), transpose per repeat (±24 st), velocity scale per repeat (10–100%), feedback-to-input toggle. Echoed notes allocate real voices — document the polyphony interaction (echoes count against the voice cap in Vintage mode).

---

## 5. Sizzler

Three modes sharing one target-set abstraction (a saved list of parameter IDs + min/max ranges):
**Mutate** — one-shot constrained randomization, amount 0–100% (interpolate current → random-in-range). **Morph** — two stored patch snapshots, CC/macro sweeps between them (interpolate in parameter space; snap stepped params at 50% crossings). **Drift** — per-target low-rate random walk (0.01–2 Hz) for evolving textures; freeze CC latches current values into the patch.

---

## 6. Generator Templates (seed data)

Each category = algorithm weights + parameter ranges + envelope archetype + naming wordlists. Starter definitions:

| Category | Algorithms (weighted) | Key traits |
|----------|----------------------|------------|
| Pads | 5, 7, 8 | AR 5–14, RR 2–5, ratios 0.5–2, detune spread, chorus on, LFO delay high |
| Bass | 1, 3, 4 | OP1 ratio 0.5–1, high feedback, fast D1R, W2/W3 waveforms in TX mode |
| Bells/Keys | 3, 5 | inharmonic ratios (1 : 3.5 : 7…), long D2R, velocity sens 4–7 |
| Percussion | 1, 2 | AR 31, RR 8–15, fixed-freq noise-ish ops, pitch EG down-sweeps |
| Sci-Fi | 2, 4, 6 | fixed frequency, S&H LFO, extreme PMD, Sizzler-drifted |
| SFX | any | feedback 6–7, pitch EG extremes, MEP4 echo presets attached |
| Fantasy / RPG | 3, 5, 7 | bell/pluck hybrids, breathy low-ratio pads, modal microtunings |
| Ambient | 5, 7, 8 | Drift-enabled, long RR, SPX90 reverse-gate presets attached |
| General MIDI | per-program | 128 fixed templates mapping GM program → nearest 4-op archetype |

Scoring pass renders 500 ms offline: reject if peak < −40 dBFS, if spectral centroid is degenerate (silent/DC), or if release clicks. Output: native preset bank + VMEM .syx (32 per bank; GM ships as 4 banks).

---

## 7. Test & Acceptance Summary

Round-trip byte equality on all sysex fixtures; null test of engine against recorded hardware phrases (tolerance defined per phrase, not bit-exact); every parameter reachable via host automation, NRPN, and UI with identical ranges; drum rack choke groups verified with overlapping-note MIDI test files; generator banks load on real TX81Z without error (manual hardware test each release).

---

## 8. Engine & DSP architecture (added)

The neutral `Patch` drives a hardware-accurate voice; **ymfm** is the reference core.

- **Core mapping.** TX81Z = **OPZ (YM2414)**; DX21/DX27/DX100 are the same 4-op algorithm family with a smaller feature set (no per-op waveforms, no fixed-frequency, no reverb). Decide: (a) one OPZ-based core parameterized down for DX21 mode, or (b) OPM for DX21 + OPZ for TX81Z. Prefer (a) — a single OPZ core with a "DX21 mask" (force sine waves, disable fixed-freq/ACED) — so voice behavior and sysex round-trip share one path. **[verify]** OPZ waveform tables and EG rate curves against ymfm + a hardware null test.
- **Vintage vs Modern engine modes** (the Settings toggle needs defined semantics):
  - *Vintage* — 8-voice polyphony, authentic envelope rate quantization and 12-bit-DAC-style output character, ymfm at 1× (optionally 2×) with the era's aliasing left largely intact. This is the "sounds like the box" mode.
  - *Modern* — higher polyphony (16/32, define the cap), full-resolution envelopes, forced oversampling, optional anti-alias filtering. Document exactly which artifacts each mode keeps/removes; users will A/B them.
- **Oversampling** 1×/2×/4× applies to the whole voice+FX path; use a polyphase up/downsampler (JUCE `dsp::Oversampling`). State the CPU multiplier per step in the UI tooltip.
- **Frequency/ratio.** Store the full **64-entry coarse ratio table** and the TX81Z fine table in `resources/` (§3 [verify]); the engine indexes them — never compute ratios ad hoc (the prototype's `parseRatio` is a shortcut to retire).
- **Null-test harness.** Render the same VCED patch through ymfm and through recorded hardware phrases; keep golden WAVs (§16). The Web Audio engine is *not* part of this null test — it's only a plausibility sketch.

## 9. Real-time safety, threading & performance budget (added)

Absent from the original handoff and essential for a shippable plugin.

- **`processBlock` is allocation-free and lock-free.** No `new`/`malloc`, no mutexes, no sysex parsing, no file I/O, no logging on the audio thread. Preallocate the voice pool and all DSP buffers.
- **Thread boundaries.** (1) audio thread — DSP only; (2) message thread — UI, librarian DB, sysex file parse/serialize, `.syx` MIDI transfer; (3) MIDI input — parameter/NRPN/sysex ingestion. Cross into the audio thread only via lock-free FIFOs (`AbstractFifo`) and atomics/APVTS. Sysex bank receive builds a `Patch` off-thread, then hands a ready voice/patch pointer across atomically.
- **Parameter model.** Every NRPN/CC target is an APVTS parameter (§4.2); the WebView bridge (if path A) marshals param changes over the JS bridge on the message thread, never touching the audio thread directly.
- **Performance budget.** Target ≤ ~1–2% CPU per Vintage voice at 1× on a modern core; define the Modern-mode budget after profiling. Meter/animation data (envelope followers, op-pulse) is pushed from the audio thread via a lock-free ring at ~30–60 Hz, read by the UI — not pulled.
- **Denormals** flushed (FTZ/DAZ); **parameter smoothing** on all continuously-varied params to avoid zipper noise (the prototype already smooths via `setTargetAtTime`; port the intent).

## 10. Plugin state, persistence & presets (added)

- **Plugin state** = current `Patch` + engine/FX/MIDI settings + a schema `version` int, serialized via `getStateInformation`/`setStateInformation` (APVTS `state.toXml` or a compact binary). Must survive DAW save/recall and be forward/backward tolerant (migrate on load by version).
- **Librarian DB scope.** The SQLite/JSON library is a **user-global** store (banks/tags/search), not per-plugin-instance — pick an OS-appropriate app-data path; a plugin instance references banks by id. Decide behavior when the DB is absent/locked by another instance (read-only fallback).
- **Preset format.** Native preset = the neutral `Patch` (JSON) so it's diffable and generator-writable; `.syx` (VCED/VMEM) is the interchange format. Support host preset systems (VST3 preset, AU factory presets) mapping to native presets.
- **Migration policy.** Bump `version` whenever the `Patch` model changes; keep pure up-migration functions and a fixture per old version in `tests/`.

## 11. Drum engine — FM rack + RX21 sample player (added)

The brief specifies **two** drum sources; the handoff models only the rack.

- **FM drum rack** — 32 slots, each a full 4-op `Patch` played at a fixed key with choke groups, per-slot level/pan/tune, group buses. Reuses the voice engine; no new DSP. This is the shippable, original-content path.
- **RX21 "authentic '85"** — the RX21 was a **PCM sample** drum machine (12-bit). Authentic mode implies sample playback + 12-bit requantization/decimation character, which is a *different engine* (sample voice, not FM) and a **copyright surface** (see §14 — do not ship Yamaha's RX21 ROM samples). Options: (a) commission/synthesize original 12-bit-flavored one-shots, (b) generate RX21-style hits from the FM engine and bake them, (c) ship the "authentic '85" toggle as a 12-bit *character* processor over FM drums without claiming the ROM. Decide before building the sample voice.

## 12. Microtuning & tuning import (added)

Called for by the brief (TX81Z micro tuning; Fantasy/RPG "modal microtunings") but unspecified.

- Support the TX81Z's octave (12-note) and full-keyboard (128-note) micro-tuning tables; store in `resources/` and round-trip via sysex where applicable **[verify format]**.
- Import **Scala `.scl`/`.kbm`** and **AnaMark `.tun`**; expose **MTS-ESP**/MIDI Tuning Standard so DAWs and other plugins can share tuning.
- Generator "modal microtunings" reference named tables from this set.

## 13. FX fidelity scope (added)

The FX section (§ design) needs an explicit ambition level — the handoff leaves DSP unspecified.

- **FX500 chain** (COMP/DIST/EQ/PITCH/DELAY) and **SPX90** (reverb/echo/gate programs) — decide **authentic-emulation vs. good-generic-DSP** per block. Recommend generic-but-tasteful DSP v1 (JUCE `dsp` modules), tuned to the era's voicing, with authentic emulation as a later goal. State this so nobody over-invests early.
- Each FX param is APVTS-automatable and NRPN-addressable (§4.2 MSB 6/7). Bypassed block = true bypass with latency compensation if any block adds latency (PITCH/DELAY will).
- SPX90 programs and FX500 block sets ship as data tables, not hardcode.

## 14. Legal / IP — factory content & trademarks (added — read before shipping) ⚠️

The original handoff omits this entirely; it is the highest-risk gap.

- **Do not ship Yamaha factory voice banks.** DX21/TX81Z ROM voices are Yamaha's copyright. Community `.syx` archives are fine as **local test fixtures** for round-trip work, but they must **not** be committed to a public repo or bundled in `resources/`. Ship **original** factory content (the generator can produce it) or clearly user-supplied banks only.
- **Do not ship RX21 samples** (§11) — same reason. Original or synthesized only.
- **Trademarks.** "DX21", "TX81Z", "RX21", "SPX90", "FX500", "MEP4", "Yamaha" are Yamaha marks. Use them **nominatively/descriptively** ("emulates the… ", "compatible with…"), not as the product name or in a confusing way. Get the product's own name/branding reviewed. Consider renaming internal engine/FX classes to neutral names in shipped strings.
- **ymfm** is BSD-3 (attribution in About/licenses). Audit every other dependency's license before release; produce a bundled `THIRD_PARTY_LICENSES`.
- **Fixtures policy.** `tests/fixtures/*.syx` stays out of version control (`.gitignore`) or in a private, non-distributed location; CI pulls them from a private bucket, not the public repo.

## 15. Build, platforms, packaging & CI (added)

- **Platforms/formats.** VST3 + AU (macOS) + CLAP + Standalone are in §1; decide on **AAX** (needs Avid SDK/signing — likely defer), **Linux** VST3/CLAP (yes if cheap), and the macOS **universal (arm64 + x86_64)** requirement.
- **macOS release chores** the scaffold omits: codesign + **notarization** + stapling for AU/VST3/Standalone; hardened runtime; a signed installer (`.pkg`) and the AU component in the right place. Windows: signed installer. 
- **CI/CD.** GitHub Actions matrix (mac-arm, mac-x64 or universal, win-x64, linux-x64) → build all formats, run pluginval + tests (§16), produce artifacts. Cache JUCE/ymfm. Gate merges on green.
- **Versioning/reproducibility.** Single source of version (CMake), embedded in plugin + About; deterministic factory-bank/tuning-table generation checked by hash.

## 16. Expanded test & validation strategy (added; extends §7)

- **pluginval** at strictness 8–10 in CI for every format (state, automation, threading, fuzzed params) — the industry gate for "loads cleanly in DAWs."
- **Sysex parser fuzzing.** The parser consumes untrusted `.syx` files — fuzz it (libFuzzer/AFL) for crashes/OOB; it must never crash or allocate unboundedly on malformed input. Security-relevant, not just correctness.
- **Golden DSP regression.** Render fixed patches/phrases to WAV; compare against committed goldens within tolerance; regenerate deliberately. Covers Vintage vs Modern, each oversample rate, each FX block.
- **Thread-safety.** ThreadSanitizer build in CI; an offline "hammer" test driving param automation + MIDI + sysex receive concurrently.
- **Round-trip** (§7) stays the sysex acceptance gate; add per-schema-version state migration tests (§10).
- **Automation parity.** Property test: for every parameter, UI-set == NRPN-set == host-automation-set produce identical engine state and identical rendered output.

## 17. Phased roadmap (mirror of the plan; see `fm-synth-plugin-plan.md` §7) (added)

Milestones, each independently demoable (authoritative version, with exit gates and sizing, lives in the companion plan §7):

- **M0 — Scaffold & CI.** §1 tree, JUCE+ymfm building all formats, pluginval green on an empty plugin, fixtures pipeline (§14) wired.
- **M1 — Voice engine + Voice UI.** OPZ core via ymfm, neutral `Patch`, ratio tables, ADSR/LFO, DX21/TX81Z mode mask. UI = webview reuse (path A) or native Voice view. **Playable, sounds right.** Null-test harness stood up.
- **M2 — Sysex I/O + Librarian.** VCED/ACED/VMEM codecs, round-trip fixtures passing, `.syx` file + MIDI send/receive, library DB, bank grid. Now it edits real hardware.
- **M3 — FX + Drums.** FX500/SPX90 (generic DSP v1), FM drum rack + choke groups; RX21 "character" mode (no ROM samples).
- **M4 — MIDI depth.** Full NRPN address space, CC learn, APVTS automation parity, MEP4 echo, Sizzler (mutate/morph/drift), microtuning + MTS.
- **M5 — Generator.** Templates/mutator/scorer/namer, offline scoring, bank export; original factory content bank.
- **M6 — Hardening & release.** Perf budget met, TSan clean, notarized signed installers, third-party licenses, trademark/branding review, hardware-load smoke on a real TX81Z.

## 18. Open decisions & [verify] log (added)

Consolidated so nothing hides in prose:

- **UI layer:** WebView reuse (A) vs native JUCE (B) — §0. *Owner: architecture. Decide before M1.*
- **DSP core:** single OPZ-parameterized core vs OPM+OPZ split — §8. *Before M1.*
- **Vintage/Modern semantics** and poly caps — §8. *Before M1.*
- **RX21 authentic mode:** original samples vs FM-baked vs character-only — §11, §14. *Before M3.*
- **FX ambition:** generic vs authentic emulation — §13. *Before M3.*
- **AAX / Linux / universal-binary** targets — §15. *Before M0 CI matrix locks.*
- **Still-[verify] byte/table details:** VCED op order (§2.1), ACED field order (§2.2), VMEM bit-packing (§2.3), 64-entry ratio + fixed-freq tables (§3), OPZ waveform/EG curves (§8), micro-tuning sysex format (§12) — all gated by fixtures/hardware, not memory.
- **Resolved:** algorithm routings 3 & 4 (§3, verified against sc-tx81z and applied to the prototype).
