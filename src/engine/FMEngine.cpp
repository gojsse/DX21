#include "FMEngine.h"
#include "Voice.h"

FMEngine::FMEngine() {
  for (auto& voice : voices_) {
    voice = std::make_unique<Voice>();
  }
}

FMEngine::~FMEngine() = default;

void FMEngine::prepare(double sampleRate, int blockSize) {
  sampleRate_ = sampleRate;
  blockSize_ = blockSize;
  for (auto& voice : voices_) {
    voice->prepare(sampleRate);
  }
}

void FMEngine::release() {
  // RT-safe cleanup
}

void FMEngine::processBlock(float* outL, float* outR, int numSamples, const juce::MidiBuffer& midi) {
  // TODO: process MIDI, call voice->noteOn/noteOff/renderBlock
  // Zero output for now
  std::fill_n(outL, numSamples, 0.0f);
  std::fill_n(outR, numSamples, 0.0f);
}

void FMEngine::setPatch(const Patch& patch) {
  currentPatch_ = patch;
  for (auto& voice : voices_) {
    voice->setPatch(patch);
  }
}

Patch FMEngine::getPatch() const {
  return currentPatch_;
}

void FMEngine::setParameter(int paramIndex, float value) {
  // TODO: update currentPatch and apply to active voices
}

void FMEngine::setEngineMode(EngineMode mode) {
  mode_ = mode;
  // TODO: reconfigure oversampling/poly cap
}

Voice* FMEngine::findFreeVoice() {
  for (auto& voice : voices_) {
    if (!voice->isActive()) return voice.get();
  }
  stealOldestVoice();
  return voices_[0].get(); // TODO: proper voice stealing
}

void FMEngine::stealOldestVoice() {
  // TODO: find oldest voice and silence it
}
