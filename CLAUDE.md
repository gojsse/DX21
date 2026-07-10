# OP4 Development Notes

**Current phase:** M1 (Voice engine + Voice UI) · started 2026-07-10

## M1 progress (in flight)

- **OPZ core wired.** `src/engine/OPZChip.{h,cpp}` wraps ymfm's `ym2414` (the OPZ = TX81Z's actual chip). One chip = 8-voice pool. Programs a `Patch` onto a channel (register-accurate: banked OW/FINE + EGS/REV, key-on gated via reg 0x08), does MIDI note→keycode, renders + resamples chip-rate→host-rate. **JUCE-free**, so it builds and runs standalone.
- **Verified by `tests/engine_tests.cpp`** (JUCE-free): render is audibly non-silent, and per-algorithm output level tracks carrier count exactly matching the Yamaha 4-op chart (algs 1–4=1 carrier, 5=2, 6–7=3, 8=4). Keycode invariants pass.
- **Engine architecture corrected:** ymfm emulates the *whole chip*, not one voice. `FMEngine` now owns the chip; `Voice` is a channel-allocation record (oldest-note stealing). `FMEngine::processBlock` does sample-accurate MIDI. `PluginProcessor::processBlock` renders live.
- **WebView editor + JS↔native bridge:** `src/PluginEditor.{h,cpp}` hosts the React UI via `juce::WebBrowserComponent` with native integration. Bridge uses an event bus: JS emits `op4_applyPatch` (canonical patch JSON) → `OP4Processor::applyPatchJson` merges onto the patch → engine; native delivers the patch via `withInitialisationData` / `emitEventIfBrowserIsVisible`. Wire format = canonical native Patch (`src/bridge/PatchJson.{h,cpp}`), **round-trip unit-tested** (`op4_patchjson_tests`). Web adapter `src/plugin/bridge.ts` (additive, no-op in browser) bridges algorithm/feedback both ways; operator/LFO/fn display↔DSP mapping is the follow-up. JS↔native round-trip needs DAW verification (can't automate the WebView JS locally).
- **Self-contained UI (embedded resources):** CMake globs `dist/` and embeds it as `juce_add_binary_data` (`OP4WebUI`); the editor serves it via `WebBrowserComponent::Options::withResourceProvider` (basename→BinaryData lookup, MIME by extension) from `getResourceProviderRoot()`. No dev-server dependency. Falls back to the dev server if `dist/` is absent or the platform lacks a resource provider, so the C++ build stays node-optional. CI runs `npm ci && npm run build` before configuring so all platforms embed + exercise the ResourceProvider. **Note:** index.html still pulls Google Fonts over the network (self-hosting fonts is a polish item).
- **`[verify]` (M1 exit gate, needs golden-WAV null-test vs. real hardware):** absolute octave/tuning reference, DX output-level→TL curve, coarse-ratio→MUL table, op→chip-slot order. All centralized as named helpers in OPZChip; carrier-count evidence already suggests slot/algorithm mapping is right.
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
