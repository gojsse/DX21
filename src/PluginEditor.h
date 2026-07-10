#pragma once
#include <juce_audio_processors/juce_audio_processors.h>  // AudioProcessorEditor
#include <juce_gui_extra/juce_gui_extra.h>                // WebBrowserComponent

class OP4Processor;

// OP4Editor — M1 WebView spike. Hosts the React prototype (/src web UI) inside
// the plugin via juce::WebBrowserComponent. In debug it loads the Vite dev
// server; shipping builds will bundle the built /dist as embedded resources and
// serve them through a ResourceProvider (JUCE 8) — see resized()/ctor TODOs.
class OP4Editor : public juce::AudioProcessorEditor {
public:
  explicit OP4Editor(OP4Processor&);
  ~OP4Editor() override;

  void paint(juce::Graphics&) override;
  void resized() override;

private:
  OP4Processor& processor_;
  juce::WebBrowserComponent web_;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OP4Editor)
};
