#pragma once
#include <juce_core/juce_core.h>
#include "../model/Patch.h"

// WebPatch — maps the web UI's *display* patch model (src/state/types.ts:
// string ratios/levels/detune, "W2 SQR" waves, 0..1 ADSR graph) onto the native
// DSP Patch. The mapping lives here in C++ so it is unit-testable
// (tests/webpatch_tests.cpp); the web adapter (src/plugin/bridge.ts) is just a
// pipe that ships its store patch over the WebView event bus.
//
// Several conversions (ratio->coarse, output-level->TL, ADSR time->rate) are
// first-pass approximations marked [verify] — they pair with the OPZChip DSP
// calibration and get locked by the null-test `compare` against hardware.
namespace op4::webpatch {

// Overlay the fields present in a web-shaped patch var onto a native Patch.
void applyWebVar(Patch& p, const juce::var& web);

// Inverse: build a (partial) web display patch from a native Patch — the native-
// owned fields only, so the web adapter can deep-merge it and keep UI-only
// fields (per-op name, sizzler, ...). Round-trips with applyWebVar for the
// reachable fields (see tests).
juce::var toWebVar(const Patch& p);

}  // namespace op4::webpatch
