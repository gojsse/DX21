#pragma once
#include <juce_core/juce_core.h>
#include "../model/Patch.h"

// PatchJson — the canonical wire format for the WebView bridge: the native DSP
// Patch <-> JSON (juce::var). The native model is the source of truth; the web
// adapter translates its display model to/from this shape.
//
// overlay() applies only the keys present in the incoming var onto an existing
// Patch, so a partial message from the UI (e.g. just "algorithm") is safe and
// never zeroes unspecified fields.
namespace op4::patchjson {

juce::var    toVar(const Patch& p);          // full canonical object
void         overlay(Patch& p, const juce::var& v);  // apply present keys only
Patch        fromVar(const juce::var& v);    // overlay onto a default Patch

juce::String encode(const Patch& p);         // JSON string
Patch        decode(const juce::String& json);  // parse -> fromVar

}  // namespace op4::patchjson
