#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"

// TODO: M1 — integrate React web UI via WebBrowserComponent or choc::ui::WebView
// For now, a stub editor

class OP4Editor : public juce::AudioProcessorEditor {
public:
  explicit OP4Editor(OP4Processor&);
  ~OP4Editor() override = default;

  void paint(juce::Graphics&) override;
  void resized() override;

private:
  OP4Processor& processor;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OP4Editor)
};
