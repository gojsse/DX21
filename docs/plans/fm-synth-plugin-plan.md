# OP4 — FM Synth Plugin Plan (master plan)

**Status:** living document · last updated 2026-07-09
**Companions:** [`op4-design-brief.md`](./op4-design-brief.md) (design vision & views) · [`op4-technical-handoff.md`](./op4-technical-handoff.md) (data formats, tables, parameter models) · [`design_handoff_op4_synth/`](./design_handoff_op4_synth/) (high-fidelity UI reference)

This is the connective document: **what we're building, for whom, in what order, and what "done" means.** It does not repeat the brief's aesthetics or the handoff's byte tables — it points to them and owns scope, architecture-at-a-glance, the roadmap, risk, and acceptance.

---

## 1. Vision

*"That synth, finally with a face."* OP4 is a software instrument — plugin + standalone — recreating Yamaha's 1985–87 4-operator FM world (DX21 / TX81Z, an RX21-style drum engine, vintage FX, a sysex librarian, a patch-bank generator). Its identity is two jobs done well:

1. **A great-sounding, legible 4-op FM instrument** — every parameter of a patch on one screen, with the algorithm selector as the tactile centerpiece. It fixes the original hardware's one scar: 100+ parameters behind a 2-line LCD.
2. **A hardware companion** — it edits and librarians *real* DX21/TX81Z units over MIDI (echo-to-hardware, sysex send/receive, bank management). Many buyers own the metal; this is the editor they never had.

The pairing is the moat: plenty of FM plugins exist; few are also a first-class librarian/editor for the actual boxes, and fewer make 4-op programming this legible.

## 2. Goals & non-goals (v1.0)

**Goals**
- Authentic-*enough* 4-op FM voice (ymfm-based, null-tested against hardware within tolerance), not a generic FM synth.
- All six views from the design brief, resizable, three themes, one-screen patch editing.
- **Full sysex round-trip** (VCED/ACED/VMEM) — edit and manage real hardware. This is the differentiator; it is not optional for 1.0.
- Depth: FX (FX500 + SPX90), FM drum rack, full MIDI/NRPN/CC-learn + host automation parity, Sizzler, microtuning, generator.
- Formats: VST3 + AU + CLAP + Standalone; macOS (universal) + Windows, signed/notarized installers.
- Ships **100% original content** (no Yamaha ROM voices or RX21 samples — see §8, handoff §14).

**Non-goals (v1.0)** — explicitly deferred so scope stays shippable:
- 6-operator / DX7 support.
- AAX / Pro Tools (revisit post-1.0).
- Bit-exact hardware emulation (we target convincing + null-tested, not sample-identical).
- iOS / AUv3, mobile.
- Cloud patch sharing, account systems.

## 3. Target users & core use cases

- **The hardware owner** — edits a real DX21/TX81Z from a legible screen; receives/sends banks; keeps a searchable library. *Primary; drives the librarian + echo-to-hardware.*
- **The in-the-box producer** — wants that FM character without the metal; plays and tweaks patches, uses the generator for fresh banks. *Drives the plugin voice + generator + FX.*
- **The FM-curious** — learns algorithms and operators because the UI makes them visible. *Drives the algorithm selector + DX21⇄TX81Z teaching toggle.*

## 4. Product scope by version

| Area | Alpha (MVP) | 1.0 | Post-1.0 |
|------|:---:|:---:|:---:|
| 4-op voice engine (ymfm), DX21/TX81Z modes | ✅ | ✅ | + more machines (DX11/V50) |
| Voice view (full patch on one screen) | ✅ | ✅ | |
| Sysex round-trip + Librarian + hardware I/O | ✅ | ✅ | |
| Remaining 5 views wired to real state | partial | ✅ | |
| FX (FX500 + SPX90) | — | ✅ generic DSP | authentic emulation |
| FM drum rack + choke groups | — | ✅ | |
| RX21 "authentic '85" character | — | ✅ (original content) | |
| Full NRPN/CC-learn + automation parity | — | ✅ | |
| Sizzler (mutate/morph/drift) | — | ✅ | |
| Microtuning (Scala/.tun/MTS) | — | ✅ | |
| Generator (templates→scored banks) | — | ✅ | smarter scoring/naming |
| Signed/notarized installers, all formats | — | ✅ | AAX, AUv3/iOS |

**MVP thesis:** the smallest thing worth shipping to alpha testers is *a playable, great-sounding 4-op voice + the Voice view + working sysex/librarian.* That alone delivers both halves of the vision (instrument + hardware editor). Everything else is 1.0 completeness.

## 5. Architecture at a glance

Full detail in the technical handoff; the shape:

- **Neutral `Patch` model is the hub** (handoff §1). Codecs (VCED/ACED/VMEM ↔ `Patch`), the engine, generator, librarian, and UI all speak `Patch` — nothing but the codecs touches raw sysex.
- **DSP = ymfm** (handoff §3, §8): one OPZ-based core with a "DX21 mask." *Vintage* (8-voice, authentic character) vs *Modern* (higher poly, oversampled) modes are defined in handoff §8.
- **UI = undecided, spike first** (handoff §0): **path A** reuses the existing React prototype in a WebView; **path B** re-implements natively in JUCE. Recommendation: spike A at M1, keep B as fallback.
- **What already exists:** a complete web prototype (`/src`) — six views, three themes, live controls, undo, and a Web Audio FM sketch. It is the UI/interaction source of truth and a DSP oracle, **not** the shipping engine.
- **RT-safety & threading** (handoff §9): audio thread does DSP only; sysex/DB/file work off-thread; lock-free hand-off.

## 6. Current status (2026-07-09)

- ✅ **Web prototype shipped** in this repo: all six views pixel-faithful to the design handoff, three runtime themes, interactive vector controls (draggable knobs & ADSR, editable values), undo, computer-keyboard-playable **Web Audio 4-op FM engine**.
- ✅ **Algorithm routings verified** (all 8) against the TX81Z matrix; algs 3 & 4 corrected.
- ⬜ No C++ / JUCE / ymfm work started. No sysex codecs. No hardware I/O.
- ⬜ UI-architecture decision (A vs B) open.
- ⬜ `fm-synth-plugin-plan.md` (this doc) now exists; the technical handoff's §17 roadmap is superseded by §7 below.

## 7. Roadmap (authoritative)

Each milestone is independently demoable and has an exit gate. Sizing is T-shirt (relative), not calendar.

### M0 — Scaffold & CI *(S)*
JUCE 8 + ymfm building all four formats from `CMakeLists`; empty plugin passes **pluginval**; fixtures pipeline wired (private, per handoff §14); GitHub Actions matrix (mac-arm/x64, win, linux) green.
**Exit:** empty plugin loads in a DAW on all platforms; CI green.

### M1 — Voice engine + Voice UI *(L)* → **playable, sounds right**
OPZ core via ymfm, neutral `Patch`, 64-entry ratio table, ADSR/LFO, DX21/TX81Z mask. **Spike UI path A** (React-in-WebView) with the param bridge; wire the Voice view to live parameters. Stand up the null-test harness (render vs. hardware phrases).
**Exit:** play a patch from a MIDI keyboard; Voice view edits it live and sounds convincingly 4-op; UI A/B decision recorded.

### M2 — Sysex I/O + Librarian *(L)* → **edits real hardware** · *ships as Alpha*
VCED/ACED/VMEM codecs with round-trip fixtures passing; `.syx` file import/export; MIDI send/receive with device number + progress; library DB + bank grid + audition; echo-to-hardware.
**Exit:** receive a bank from a real TX81Z, edit a voice, send it back; all round-trip fixtures byte-identical. **→ private alpha.**

### M3 — FX + Drums *(M)*
FX500 chain + SPX90 (generic DSP v1, handoff §13); FM drum rack + choke groups; RX21 character mode (no ROM samples).
**Exit:** FX/drum views fully wired; choke-group MIDI tests pass.

### M4 — MIDI depth *(M)*
Full NRPN address space, CC-learn, APVTS automation with **UI == NRPN == host** parity; MEP4 echo; Sizzler (mutate/morph/drift); microtuning + MTS.
**Exit:** every parameter reachable & identical via three paths; Sizzler + tuning demoable.

### M5 — Generator *(M)*
Templates / mutator / scorer / namer; offline scoring; bank export; **original** factory-content bank generated.
**Exit:** generate → audition → keep/discard → export `.syx`; scored banks load on hardware.

### M6 — Hardening & release *(M)* → **1.0**
Perf budget met (handoff §9), ThreadSanitizer clean, pluginval strict; codesign + notarize + installers; third-party licenses; **trademark/branding review**; hardware-load smoke on a real unit.
**Exit:** all §9 acceptance criteria pass; signed installers; legal review clear. **→ 1.0.**

**Critical path:** M0 → M1 → M2 is the MVP spine; M3–M5 parallelize somewhat once `Patch` + engine are stable; M6 gates release.

## 8. Cross-cutting concerns (pointers)

- **Legal / IP — read first (handoff §14).** No Yamaha factory banks, no RX21 ROM samples, nominative trademark use only, fixtures out of the public repo. This is the top non-engineering risk and blocks release, not just polish.
- **RT-safety, threading, perf** — handoff §9.
- **State/persistence/preset migration** — handoff §10.
- **Test strategy** (pluginval, sysex fuzzing, golden-WAV DSP regression, TSan, round-trip, automation parity) — handoff §16.
- **Microtuning, FX scope, drum engine split** — handoff §11–13.

## 9. Risks & mitigations

| Risk | Impact | Mitigation |
|------|--------|-----------|
| **Shipping Yamaha IP** (banks/samples/marks) | Release-blocking legal | Original content only; legal/branding review at M6; fixtures private (§14). |
| **UI-architecture bet (WebView) disappoints** | Rework, latency | Spike path A at M1 behind a gate; keep native path B as fallback; prototype de-risks before committing. |
| **ymfm ≠ hardware** (character off) | Core value miss | Null-test harness from M1; fixtures + real-unit A/B; tune before alpha. |
| **Sysex byte-detail wrong** | Corrupts users' hardware patches | Round-trip fixtures are the acceptance gate; "import anyway" for bad checksums; validate every [verify] against dumps, not memory. |
| **Scope creep** (this is a big product) | Never ships | MVP-first (M0–M2 → alpha); FX/drums/gen are 1.0, not blocking alpha. |
| **RT-safety bugs** (glitches/crashes in DAWs) | Reputation | No alloc/locks in `processBlock`; TSan + pluginval in CI from M0. |
| **Two UIs drift** (web demo + plugin) | Maintenance | If path A, one UI serves both; if B, decide whether the web demo is retired or kept as marketing only. |

## 10. Dependencies & licensing

- **JUCE 8** (framework) — confirm license tier for a commercial release.
- **ymfm** — BSD-3; attribution in About + `THIRD_PARTY_LICENSES`.
- **choc** / webview (if path A) — check license.
- **SQLite** (librarian, if chosen) — public domain.
- Audit *every* dependency's license before 1.0; generate a bundled licenses file (handoff §14).

## 11. Success criteria (1.0 acceptance)

- Loads and passes **pluginval** (strict) on VST3/AU/CLAP across mac + win; validated manually in the major DAWs.
- **Round-trips every sysex fixture** byte-identically; edits a real TX81Z over MIDI end-to-end (hardware smoke).
- Sounds convincingly like the hardware — null test within per-phrase tolerance + subjective A/B sign-off.
- Every parameter reachable and identical via UI, NRPN, and host automation.
- Ships signed/notarized installers containing **only original content**, with a clean legal/trademark review.
- Standalone + plugin recall state correctly across DAW save/reopen.

## 12. Open decisions (owner · when)

Consolidated in handoff §18; the release-shaping ones:

- **UI layer A vs B** — architecture · before/at M1.
- **DSP core:** single OPZ-masked vs OPM+OPZ split — before M1.
- **Vintage/Modern semantics + poly caps** — before M1.
- **RX21 authentic mode** (original samples vs FM-baked vs character-only) — before M3.
- **FX ambition** (generic vs authentic) — before M3.
- **AAX / Linux / universal-binary** targets — before M0 CI locks.
- **Web demo fate** if path B chosen — before M6.

## 13. Document map

- **Design brief** — the *why* and the *look*: product identity, aesthetic, the six views, interaction principles.
- **This plan** — the *what/when*: scope, versions, roadmap, risk, acceptance.
- **Technical handoff** — the *how*: sysex byte formats, algorithm/ratio tables, MIDI/NRPN maps, engine/DSP/threading/state/legal/test detail, decision log.
- **`design_handoff_op4_synth/`** — the pixel-level UI reference (`.dc.html` + screenshots).
- **`/src` prototype** — the running, validated web implementation of the UI + a Web Audio FM sketch.
