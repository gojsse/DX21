#include "PluginEditor.h"
#include "PluginProcessor.h"

juce::WebBrowserComponent::Options OP4Editor::makeOptions() {
  OP4Processor& p = processor_;  // outlives this editor; safe to capture by ref
  return juce::WebBrowserComponent::Options{}
      .withNativeIntegrationEnabled()
      // JS -> native: the UI emits canonical patch JSON (whole or partial).
      .withEventListener("op4_applyPatch",
          [&p](juce::var payload) {
            const juce::String json = payload.isString() ? payload.toString()
                                                         : juce::JSON::toString(payload);
            p.applyPatchJson(json);
          })
      // native -> JS at load: current patch as a JSON string.
      .withInitialisationData("op4_patch", p.getPatchJson());
}

OP4Editor::OP4Editor(OP4Processor& p)
    : juce::AudioProcessorEditor(&p), processor_(p), web_(makeOptions()) {
  addAndMakeVisible(web_);

  // M1 spike: load the Vite dev server. Native integration still injects the
  // window.__JUCE__ bridge into this page.
  // TODO(M1): bundle `npm run build` output (/dist) as BinaryData and serve it
  // via Options().withResourceProvider(...) so the plugin is self-contained.
  web_.goToURL("http://localhost:5173");

  setResizable(true, true);
  if (auto* c = getConstrainer())
    c->setSizeLimits(900, 560, 2400, 1520);
  setSize(1200, 760);  // matches the prototype's 1200px reference scale
}

OP4Editor::~OP4Editor() = default;

void OP4Editor::pushPatch() {
  // native -> JS: deliver the current patch as an object the web adapter applies.
  // TODO(M1): call this when the host changes state/automation so the UI tracks.
  web_.emitEventIfBrowserIsVisible("op4_patch", juce::JSON::parse(processor_.getPatchJson()));
}

void OP4Editor::paint(juce::Graphics& g) {
  g.fillAll(juce::Colours::black);
}

void OP4Editor::resized() {
  web_.setBounds(getLocalBounds());
}
