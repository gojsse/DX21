# OP4 Development Notes

**Current phase:** M0 (Scaffold & CI) → M1 (Voice engine + Voice UI) · started 2026-07-10

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

## Next immediate steps (M0)

1. Create `CMakeLists.txt` with JUCE 8 + ymfm as FetchContent, targets: VST3, AU, CLAP, Standalone.
2. Wire `libs/ymfm/` as a git submodule.
3. Set up GitHub Actions matrix (mac-universal, win-x64, linux-x64) → build all formats, run pluginval.
4. Verify empty plugin loads in a DAW on macOS + Windows.

## Build notes (macOS 15 workaround)

**Local builds on macOS 15:** JUCE 7.0.12 (latest stable) has a deprecated API issue (`CGWindowListCreateImage`). The CMakeLists sets `JUCE_BUILD_TOOLS OFF` but juceaide is still built as a dependency.

**Workaround for local testing:**
- Use **GitHub Actions CI** (macos-13 or ubuntu-latest) — no issue there.
- Or build only the plugin target: `cmake --build build --target OP4_Standalone 2>&1 | grep -v juceaide`.
- JUCE 8.2.0+ (when released) will fix this; M1+ work can proceed on Linux CI in the meantime.

**Impact:** M0 scaffold is structurally complete; M1 DSP work can proceed with CI builds while local Xcode builds sort out the deprecation issue.

## Decisions to make before code touch-down

None — the UI path, DSP core, and roadmap are locked. Start building.
