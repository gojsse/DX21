#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <functional>
#include "engine/FMEngine.h"
#include "model/Patch.h"
#include "sysex/SysexFifo.h"
#include "sysex/SysexRouter.h"
#include "sysex/VMEMCodec.h"

class OP4Processor : public juce::AudioProcessor, private juce::Timer {
public:
  OP4Processor();
  ~OP4Processor() override;

  void prepareToPlay(double sampleRate, int samplesPerBlock) override;
  void releaseResources() override;
  void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

  juce::AudioProcessorEditor* createEditor() override;
  bool hasEditor() const override { return true; } // WebView editor (M1 spike)

  const juce::String getName() const override { return "OP4"; }
  bool acceptsMidi() const override { return true; }
  bool producesMidi() const override { return true; }  // transmits voice sysex out
  bool isMidiEffect() const override { return false; }
  double getTailLengthSeconds() const override { return 0.0; }

  int getNumPrograms() override { return 1; }
  int getCurrentProgram() override { return 0; }
  void setCurrentProgram(int) override {}
  const juce::String getProgramName(int) override { return "Default"; }
  void changeProgramName(int, const juce::String&) override {}

  void getStateInformation(juce::MemoryBlock& destData) override;
  void setStateInformation(const void* data, int sizeInBytes) override;

  // WebView bridge (message thread). applyPatchJson merges canonical patch JSON
  // (used for state persistence); applyWebPatch maps the UI's display-model
  // patch var onto the current patch. Both push the result to the engine.
  void applyPatchJson(const juce::String& json);
  void applyWebPatch(const juce::var& webPatch);
  juce::String getPatchJson() const;
  juce::var getWebPatch() const;  // native patch -> web display var (for the UI)

  // Load a .syx blob (message thread): split into messages, route, and load the
  // first resulting voice. Returns how many voices were found.
  int loadSyx(const uint8_t* data, int size);

  // The current voice as ACED + VCED sysex (for file export).
  std::vector<uint8_t> getVoiceSyx() const;
  // Queue the current voice (ACED + VCED) for MIDI transmission (message thread).
  void sendVoice();

  // Loaded VMEM bank -> Library. selectBankVoice loads voice i into the engine.
  void selectBankVoice(int index);
  juce::var getBankInfo() const;  // { loaded, current, names[32] } for the UI

  // Set by the editor: called on the message thread when a patch arrives via
  // sysex, so the WebView UI can refresh. Cleared when the editor is destroyed.
  std::function<void()> onPatchLoaded;

private:
  void setPatch(const Patch& p);  // update currentPatch + engine
  void timerCallback() override;  // drain incoming sysex on the message thread

  std::unique_ptr<FMEngine> engine;
  Patch currentPatch;

  op4::SysexFifo sysexIn_;    // audio thread -> message thread
  op4::SysexFifo sysexOut_;   // message thread -> audio thread
  op4::SysexRouter router_;   // message thread only
  std::vector<uint8_t> sysexScratch_;
  std::vector<uint8_t> sysexOutScratch_;

  VMEMCodec::PatchBank bank_{};
  bool hasBank_ = false;
  int  currentVoice_ = 0;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OP4Processor)
};
