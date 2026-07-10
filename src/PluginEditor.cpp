#include "PluginEditor.h"
#include "PluginProcessor.h"

OP4Editor::OP4Editor(OP4Processor& p)
    : juce::AudioProcessorEditor(&p), processor_(p) {
  addAndMakeVisible(web_);

  // M1 spike: point the WebView at the running Vite dev server so the real
  // React UI renders inside the plugin window. This proves the WebView path
  // (architecture path A) end-to-end.
  //
  // TODO(M1): bundle `npm run build` output (/dist) as BinaryData and serve it
  // via juce::WebBrowserComponent::Options().withResourceProvider(...) so the
  // plugin is self-contained (no dev server). Then stand up the parameter
  // bridge: JS -> native (control edits) and native -> JS (host automation)
  // over WebBrowserComponent's native function / event channel.
  web_.goToURL("http://localhost:5173");

  setResizable(true, true);
  if (auto* c = getConstrainer())
    c->setSizeLimits(900, 560, 2400, 1520);
  setSize(1200, 760);  // matches the prototype's 1200px reference scale
}

OP4Editor::~OP4Editor() = default;

void OP4Editor::paint(juce::Graphics& g) {
  g.fillAll(juce::Colours::black);
}

void OP4Editor::resized() {
  web_.setBounds(getLocalBounds());
}
