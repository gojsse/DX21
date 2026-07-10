#include "PluginEditor.h"
#include "PluginProcessor.h"
#include <cstring>
#include <optional>
#include <vector>

// Serve the embedded /dist only when the UI was bundled AND the platform's
// WebView supports a resource provider; otherwise fall back to the dev server.
#if defined(OP4_HAS_EMBEDDED_UI) && JUCE_WEB_BROWSER_RESOURCE_PROVIDER_AVAILABLE
 #include <BinaryData.h>
 #define OP4_EMBEDDED_UI 1
#else
 #define OP4_EMBEDDED_UI 0
#endif

#if OP4_EMBEDDED_UI
static juce::String mimeForPath(const juce::String& path) {
  if (path.endsWithIgnoreCase(".html"))  return "text/html";
  if (path.endsWithIgnoreCase(".js"))    return "text/javascript";
  if (path.endsWithIgnoreCase(".css"))   return "text/css";
  if (path.endsWithIgnoreCase(".svg"))   return "image/svg+xml";
  if (path.endsWithIgnoreCase(".json"))  return "application/json";
  if (path.endsWithIgnoreCase(".woff2")) return "font/woff2";
  if (path.endsWithIgnoreCase(".woff"))  return "font/woff";
  if (path.endsWithIgnoreCase(".png"))   return "image/png";
  if (path.endsWithIgnoreCase(".ico"))   return "image/x-icon";
  return "application/octet-stream";
}

// Map a request path to an embedded resource by basename. Vite emits a flat
// dist (index.html + assets/<hashed>.{js,css}) so basenames are unique.
static std::optional<juce::WebBrowserComponent::Resource> provideResource(const juce::String& path) {
  const juce::String name = (path == "/") ? juce::String("index.html")
                                          : path.fromLastOccurrenceOf("/", false, false);
  for (int i = 0; i < BinaryData::namedResourceListSize; ++i) {
    if (juce::String(BinaryData::originalFilenames[i]) == name) {
      int size = 0;
      const char* data = BinaryData::getNamedResource(BinaryData::namedResourceList[i], size);
      std::vector<std::byte> bytes(static_cast<size_t>(size));
      std::memcpy(bytes.data(), data, static_cast<size_t>(size));
      return juce::WebBrowserComponent::Resource{ std::move(bytes), mimeForPath(name) };
    }
  }
  return std::nullopt;
}
#endif

juce::WebBrowserComponent::Options OP4Editor::makeOptions() {
  OP4Processor& p = processor_;  // outlives this editor; safe to capture by ref
  auto opts = juce::WebBrowserComponent::Options{}
      .withNativeIntegrationEnabled()
      // JS -> native: the UI emits its display-model patch; native maps it
      // (bridge/WebPatch) onto the engine's patch.
      .withEventListener("op4_webPatch",
          [&p](juce::var payload) { p.applyWebPatch(payload); })
      // native -> JS at load: current patch as canonical JSON (partial inbound;
      // full native->display formatting is a follow-up).
      .withInitialisationData("op4_patch", p.getPatchJson());
#if OP4_EMBEDDED_UI
  opts = opts.withResourceProvider(provideResource);
#endif
  return opts;
}

OP4Editor::OP4Editor(OP4Processor& p)
    : juce::AudioProcessorEditor(&p), processor_(p), web_(makeOptions()) {
  addAndMakeVisible(web_);

#if OP4_EMBEDDED_UI
  // Self-contained: serve the embedded /dist from the resource-provider origin.
  web_.goToURL(juce::WebBrowserComponent::getResourceProviderRoot());
#else
  // Fallback (dist/ not bundled, or platform without a resource provider):
  // load the Vite dev server. Native integration still injects window.__JUCE__.
  web_.goToURL("http://localhost:5173");
#endif

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
