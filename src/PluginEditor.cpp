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
      // UI requests a .syx file load / export -> native file chooser.
      .withEventListener("op4_loadSyx",
          [this](juce::var) { openSyxChooser(); })
      .withEventListener("op4_exportSyx",
          [this](juce::var) { exportSyxChooser(); })
      // UI requests transmitting the current voice out over MIDI.
      .withEventListener("op4_sendVoice",
          [&p](juce::var) { p.sendVoice(); })
      // native -> JS at load: current patch in the UI's display model, which the
      // web adapter deep-merges into its store.
      .withInitialisationData("op4_initPatch", p.getWebPatch());
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

  // Refresh the UI when a patch arrives via sysex (message thread).
  processor_.onPatchLoaded = [this] { pushPatch(); };
}

OP4Editor::~OP4Editor() {
  processor_.onPatchLoaded = nullptr;
}

void OP4Editor::openSyxChooser() {
  chooser_ = std::make_unique<juce::FileChooser>(
      "Load a .syx patch", juce::File(), "*.syx;*.SYX");
  const auto flags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;
  chooser_->launchAsync(flags, [this](const juce::FileChooser& fc) {
    const juce::File file = fc.getResult();
    if (file == juce::File{}) return;
    juce::MemoryBlock mb;
    if (file.loadFileAsData(mb) && mb.getSize() > 0)
      processor_.loadSyx(static_cast<const uint8_t*>(mb.getData()), static_cast<int>(mb.getSize()));
  });
}

void OP4Editor::exportSyxChooser() {
  chooser_ = std::make_unique<juce::FileChooser>(
      "Export voice as .syx",
      juce::File::getSpecialLocation(juce::File::userDocumentsDirectory).getChildFile("OP4 Voice.syx"),
      "*.syx");
  const auto flags = juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles
                   | juce::FileBrowserComponent::warnAboutOverwriting;
  chooser_->launchAsync(flags, [this](const juce::FileChooser& fc) {
    const juce::File file = fc.getResult();
    if (file == juce::File{}) return;
    const auto bytes = processor_.getVoiceSyx();
    file.replaceWithData(bytes.data(), static_cast<int>(bytes.size()));
  });
}

void OP4Editor::pushPatch() {
  // native -> JS: deliver the current patch in the UI's display model. The web
  // adapter deep-merges it into its store.
  // TODO(M2): call this on host state recall / automation so the UI tracks.
  web_.emitEventIfBrowserIsVisible("op4_setPatch", processor_.getWebPatch());
}

void OP4Editor::paint(juce::Graphics& g) {
  g.fillAll(juce::Colours::black);
}

void OP4Editor::resized() {
  web_.setBounds(getLocalBounds());
}
