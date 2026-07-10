#pragma once
#include <juce_audio_processors/juce_audio_processors.h>  // AudioProcessorEditor
#include <juce_gui_extra/juce_gui_extra.h>                // WebBrowserComponent

class OP4Processor;

// OP4Editor — M1 WebView editor. Hosts the React prototype (/src web UI) via
// juce::WebBrowserComponent with native integration enabled, so a JS<->native
// bridge is available:
//   - JS -> native: the UI emits "op4_applyPatch" events (canonical patch JSON,
//     see bridge/PatchJson) which are merged onto the processor's patch.
//   - native -> JS: the initial patch is delivered via withInitialisationData;
//     runtime pushes use pushPatch() -> "op4_patch" events.
//
// The wire format is the canonical native Patch (bridge/PatchJson). The web
// adapter (src/plugin/bridge.ts) translates the UI's display model to/from it.
class OP4Editor : public juce::AudioProcessorEditor {
public:
  explicit OP4Editor(OP4Processor&);
  ~OP4Editor() override;

  void paint(juce::Graphics&) override;
  void resized() override;

  // Push the current patch to the UI (host automation / state recall). Not yet
  // called on a schedule — see TODO in the .cpp.
  void pushPatch();

private:
  juce::WebBrowserComponent::Options makeOptions();

  OP4Processor& processor_;
  juce::WebBrowserComponent web_;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OP4Editor)
};
