#pragma once
#include <memory>
#include <array>
#include "../model/Patch.h"

class Voice;

class FMEngine {
public:
  FMEngine();
  ~FMEngine();

  void prepare(double sampleRate, int blockSize);
  void release();

  void processBlock(float* outL, float* outR, int numSamples, const juce::MidiBuffer& midi);
  void setPatch(const Patch& patch);
  Patch getPatch() const;

  // RT-safe parameter update (used by APVTS)
  void setParameter(int paramIndex, float value);

  // Oversampling mode
  enum class EngineMode { Vintage, Modern };
  void setEngineMode(EngineMode mode);

  static constexpr int MaxVoices = 32;
  static constexpr int BlockAlignment = 256;

private:
  double sampleRate_ = 44100.0;
  int blockSize_ = 256;
  Patch currentPatch_;
  EngineMode mode_ = EngineMode::Vintage;

  std::array<std::unique_ptr<Voice>, MaxVoices> voices_;

  void stealOldestVoice();
  Voice* findFreeVoice();

  // Lock-free FIFO for parameter updates
  // TODO: implement when MIDI routing is wired
};
