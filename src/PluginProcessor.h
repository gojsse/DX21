#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "engine/FMEngine.h"
#include "model/Patch.h"

class OP4Processor : public juce::AudioProcessor {
public:
  OP4Processor();
  ~OP4Processor() override = default;

  void prepareToPlay(double sampleRate, int samplesPerBlock) override;
  void releaseResources() override;
  void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

  juce::AudioProcessorEditor* createEditor() override;
  bool hasEditor() const override { return true; } // WebView editor (M1 spike)

  const juce::String getName() const override { return "OP4"; }
  bool acceptsMidi() const override { return true; }
  bool producesMidi() const override { return false; }
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

private:
  std::unique_ptr<FMEngine> engine;
  Patch currentPatch;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OP4Processor)
};
