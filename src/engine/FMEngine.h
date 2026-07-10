#pragma once
#include <memory>
#include <array>
#include <cstdint>
#include <juce_audio_basics/juce_audio_basics.h>
#include "../model/Patch.h"
#include "Voice.h"

class OPZChip;

// FMEngine — the JUCE-facing voice manager. Owns the OPZ chip (one per 8-voice
// pool), turns a juce::MidiBuffer into note-on/off + channel allocation, and
// renders the block. Mono-timbral: all channels share the current patch.
class FMEngine {
public:
  FMEngine();
  ~FMEngine();

  void prepare(double sampleRate, int blockSize);
  void release();

  void processBlock(float* outL, float* outR, int numSamples, const juce::MidiBuffer& midi);

  void setPatch(const Patch& patch);
  Patch getPatch() const;

  // RT-safe single-parameter update (APVTS). TODO(M1): map paramIndex -> patch field.
  void setParameter(int paramIndex, float value);

  // DX21 family (sine-only, no fixed-freq/ACED) vs TX81Z (full).
  void setDX21Mask(bool on);

  // Oversampling / polyphony mode. Modern (multi-chip) is TODO; Vintage = 1 chip.
  enum class EngineMode { Vintage, Modern };
  void setEngineMode(EngineMode mode);

  static constexpr int kPolyphony = 8;  // one OPZ chip

private:
  void reprogramAllChannels();
  void handleNoteOn(int note, float velocity);
  void handleNoteOff(int note);
  void handleController(int cc, int value);
  void handlePitchBend(int value14);   // 0..16383, centre 8192
  void releaseSustained();
  void updateExpression();
  int  allocateChannel();

  double sampleRate_ = 44100.0;
  int    blockSize_ = 256;
  Patch  currentPatch_;
  bool   dx21Mask_ = true;
  bool   patchDirty_ = true;
  EngineMode mode_ = EngineMode::Vintage;

  std::unique_ptr<OPZChip> chip_;
  std::array<Voice, kPolyphony> voices_;
  uint64_t tick_ = 0;

  // performance controllers
  bool sustainOn_ = false;
  int  ccVolume_ = 127;      // CC7
  int  ccExpression_ = 127;  // CC11
};
