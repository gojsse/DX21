# OP4 Development Notes

**Current phase:** M1 (Voice engine + Voice UI) · started 2026-07-10

## M1 progress (in flight)

- **OPZ core wired.** `src/engine/OPZChip.{h,cpp}` wraps ymfm's `ym2414` (the OPZ = TX81Z's actual chip). One chip = 8-voice pool. Programs a `Patch` onto a channel (register-accurate: banked OW/FINE + EGS/REV, key-on gated via reg 0x08), does MIDI note→keycode, renders + resamples chip-rate→host-rate. **JUCE-free**, so it builds and runs standalone.
- **Verified by `tests/engine_tests.cpp`** (JUCE-free): render is audibly non-silent, and per-algorithm output level tracks carrier count exactly matching the Yamaha 4-op chart (algs 1–4=1 carrier, 5=2, 6–7=3, 8=4). Keycode invariants pass.
- **Engine architecture corrected:** ymfm emulates the *whole chip*, not one voice. `FMEngine` now owns the chip; `Voice` is a channel-allocation record (oldest-note stealing). `FMEngine::processBlock` does sample-accurate MIDI. `PluginProcessor::processBlock` renders live.
- **Performance controllers:** velocity→carrier TL (`carrierMask` per algorithm); **pitch bend** (fractional `computeKeyCode` → KC/KF, scaled by `pb_range`); **sustain pedal** (CC64, defers note-offs in `FMEngine`); **volume/expression** (CC7/CC11 → carrier attenuation via `setExpression`). All applied so defaults (vel 1.0, bend 0, vol/expr 127) leave the golden unchanged. Unit-tested at the OPZChip level.
- **LFO wired:** `OPZChip::programLFO` writes the chip-global LFO (rate 0x18, AM depth 0x19, PM depth 0x189 via 0x19|bit7, waveform+sync 0x1b — reg 0x1b's upper bits are external CT pins, not audio, so safe to zero). PMS/AMS stay per-channel (programChannel, 0x38). **Mod wheel (CC1)** adds LFO PM depth (`setModWheel`). Since the web UI exposes LFO *depth* but not *sensitivity*, `WebPatch` nudges PMS/AMS when PMD/AMD>0 so the UI's LFO is audible. Golden stays valid (OPZChip tests never call programLFO; default patches have depth 0). Tested: LFO modulates the tone, mod wheel adds vibrato with patch PMD 0.
- **Breath (CC2):** `src/engine/Controllers.h` (pure, JUCE-free, unit-tested) → breath→amplitude (folded into the expression gain) and breath→pitch (folded into the bend). Like the DX, breath only acts when the patch enables it (`breath_amp`/`breath_pitch` > 0), so defaults ignore CC2 and the golden is unaffected. Breath→EG-bias deferred to M4.
- **WebView editor + JS↔native bridge:** `src/PluginEditor.{h,cpp}` hosts the React UI via `juce::WebBrowserComponent` with native integration. Bridge uses an event bus: JS emits `op4_applyPatch` (canonical patch JSON) → `OP4Processor::applyPatchJson` merges onto the patch → engine; native delivers the patch via `withInitialisationData` / `emitEventIfBrowserIsVisible`. Two mappers: `PatchJson` (canonical native Patch⇄JSON, for state persistence, round-trip unit-tested) and **`WebPatch`** (`src/bridge/WebPatch.{h,cpp}`) which maps the UI's *display model* (string ratios/levels/detune, "W2 SQR" waves, 0..1 ADSR) → native DSP Patch. Mapping lives in C++ so it's unit-tested (`op4_webpatch_tests`): direct fields (lvl→tl, wave→index, det→detune incl. Unicode-minus, algorithm/feedback/poly-mono/transpose, LFO knobs) are exact; ratio→coarse, output-level→TL, and ADSR-time→rate are first-pass approximations marked `[verify]` (locked later by the null-test vs hardware). Two-way: UI edits → `op4_webPatch` → `applyWebVar` → engine; native → UI via `toWebVar` (inverse mapper, native→web→native round-trip unit-tested) delivered as `op4_initPatch` init data + `op4_setPatch` runtime push (editor `pushPatch()`; wired for M2 state recall). Web adapter deep-merges inbound so UI-only fields (per-op name, sizzler) survive. JS↔native round-trip still needs DAW verification (can't automate the WebView JS locally).
- **Plugin vs browser (`isInPlugin()`):** inside the plugin the web prototype's Web Audio *sketch* engine + computer-keyboard play + click-to-audition are **disabled** (guarded in `useAudio`/`auditionNote`) — the host's MIDI drives the real ymfm engine, and the UI is UI-only. In a plain browser they still work (the web demo). Confirmed in Ableton: the VST3 loads + responds; the on-screen keyboard/patch-audition no longer competes. **Patch switching between voices is M2** (the Library is currently a visual mockup — one real patch; no bank/sysex load yet).
- **Self-contained UI (embedded resources):** CMake globs `dist/` and embeds it as `juce_add_binary_data` (`OP4WebUI`); the editor serves it via `WebBrowserComponent::Options::withResourceProvider` (basename→BinaryData lookup, MIME by extension) from `getResourceProviderRoot()`. No dev-server dependency. Falls back to the dev server if `dist/` is absent or the platform lacks a resource provider, so the C++ build stays node-optional. CI runs `npm ci && npm run build` before configuring so all platforms embed + exercise the ResourceProvider. **Note:** index.html still pulls Google Fonts over the network (self-hosting fonts is a polish item).
- **Null-test / golden-WAV harness:** `tests/null_test.cpp` (+ `tests/wav.h`, JUCE-free). Renders a fixed phrase at the chip's **native rate bypassing the resampler** (`OPZChip::renderNative` → pure-integer ymfm, bit-deterministic cross-platform) and nulls it against committed `tests/golden/phrase_native.wav` (`op4_null_test` in ctest; −300 dB = perfect null). Also `render <wav>` / `compare a.wav b.wav` (null depth in dB) for the **hardware** null-test: put a real-TX81Z capture in `tests/fixtures/` (gitignored, legal/IP) and `compare` to lock the `[verify]` items. Run `op4_null_test golden <path> --update` after an intentional DSP change.
- **Tuning: LOCKED ✅** — `computeKeyCode` was one semitone sharp (OPM note-code off-by-one); fixed by referencing the note table from `base-1` (which also makes C wrap to the top of the octave field). Verified by measuring the engine's own sine output: A4=440.0, exact equal temperament across the range. No hardware needed.
- **`[verify]` remaining (needs docs or hardware `compare`):** DX output-level→TL curve, coarse-ratio→MUL table, **op→chip-slot order** (the sine probe exposed that isolating OP1 in alg 0 is silent → the carrier isn't slot 0; needs the OPM slot permutation, likely `{0,2,1,3}`), controller depth constants. All centralized as named helpers in OPZChip.
- **Fixed latent M0 bugs:** several CMake-referenced files never existed (`Types.h`, `LFO.h`, `Envelope.h`, `SysexRouter.h`, `CCTable.h`, `MEP4Echo.h`); both test files defined `main()` into one exe (now split into two JUCE-free test targets); `Patch::operator==` was declared-but-undefined (now C++20 defaulted).

## Architecture decisions (locked)

- **UI path: A (WebView).** Reuse the existing React prototype (`/src`) inside the JUCE plugin via a webview (JUCE 8 `WebBrowserComponent` or `choc::ui::WebView`). Fastest to a playable, great-looking plugin; one UI codebase for web demo + plugin + standalone.
- **DSP core:** single OPZ-based ymfm core with a "DX21 mask" (force sine, disable fixed-freq/ACED). Applies to all machines (DX21/27/100 → TX81Z).
- **Vintage/Modern modes:** *Vintage* = 8-voice, authentic 12-bit character, 1× ymfm; *Modern* = higher poly, oversampled, full-resolution EG.

## Files and structure

- **`docs/plans/`** — design brief, master plan, technical handoff, and status snapshots. **Canonical source for scope/roadmap/risk.** All three are linked; when they disagree, the plan (`fm-synth-plugin-plan.md`) wins.
- **`/src`** — React web prototype (production-ready UI, Web Audio FM sketch). This is the **UI source of truth** and will be embedded in the JUCE plugin (M1+).
- **`CMakeLists.txt`** (coming M0) — JUCE 8 + ymfm as FetchContent, targets VST3/AU/CLAP/Standalone.
- **`.github/workflows/`** (coming M0) — CI matrix (mac-arm/x64, win, linux), pluginval, tests.

## Known open items

- **Sysex codecs** (VCED/ACED/VMEM) — implemented as pure `Patch` ↔ binary codec functions, no JUCE AudioProcessor coupling (M2).
- **Right-click context menu** ("assign CC / add to Sizzler") — reserved in UI, not drawn yet (handoff §0 known-open).
- **RX21 authentic mode** — decide: original samples vs FM-baked vs character-only (before M3, handoff §11).
- **Legal/IP** — **top priority before release** (handoff §14): no Yamaha factory banks/ROM, fixtures private, trademark review.

## Per-session guidance

- **Planning is stable.** Refer to `docs/plans/fm-synth-plugin-plan.md` for roadmap/risk/acceptance; `op4-technical-handoff.md` for data formats/byte tables/decision log.
- **Web prototype is locked.** No UI changes unless they unblock plugin work or fix bugs. The design is complete.
- **Commit early, often.** One commit per feature/section; the roadmap is the narrative.
- **Test scope:** pluginval (CI), sysex round-trip fixtures (M2), golden-WAV DSP regression (M1), null-test harness (M1).

## Next immediate steps (M1 continued)

1. **Bundle the WebView UI:** `npm run build` → embed `/dist` as JUCE `BinaryData`, serve via `WebBrowserComponent::Options().withResourceProvider(...)` so the plugin is self-contained (no dev server).
2. **Parameter bridge:** JS→native (UI edits → `FMEngine::setPatch`/`setParameter`) and native→JS (host automation → UI) over the WebView native channel; wire APVTS.
3. **Null-test harness:** render golden phrases, compare against real-hardware WAV fixtures (private) to lock the `[verify]` calibration items (tuning, TL curve, ratio table, slot order).
4. **Velocity + controllers:** carrier-TL velocity scaling, pitch bend, sustain pedal, mod/breath.

## Build notes

- **JUCE pinned to 8.0.14** (FetchContent, `GIT_SHALLOW`). JUCE 8 resolves the macOS-15 `CGWindowListCreateImage`/juceaide deprecation that blocked local builds under 7.0.12.
- **Fast DSP/codec iteration without JUCE:** the OPZ core and sysex codecs are JUCE-free. Build+run the engine test directly:
  ```
  c++ -std=c++20 -O2 -I libs/ymfm/src -I src \
      tests/engine_tests.cpp src/engine/OPZChip.cpp libs/ymfm/src/ymfm_opz.cpp \
      -o build/engine_tests && ./build/engine_tests
  ```
- **Full plugin build:** `git submodule update --init --recursive` then `cmake -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build`. CI (macos/win/linux) is the source of truth for all four plugin formats + pluginval.

## Decisions to make before code touch-down

None — the UI path, DSP core, and roadmap are locked. Start building.
