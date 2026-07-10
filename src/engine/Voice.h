#pragma once
#include "../model/Patch.h"

class Voice {
public:
  Voice();
  ~Voice() = default;

  void prepare(double sampleRate);

  void noteOn(int noteNumber, float velocity);
  void noteOff(float velocity);
  void pitchBend(float semitones);
  void setParameter(int operatorIndex, int paramIndex, float value);

  void renderBlock(float* outL, float* outR, int numSamples);

  bool isActive() const { return active_; }
  int getAgeInSamples() const { return ageInSamples_; }
  void setPatch(const Patch& patch);

private:
  bool active_ = false;
  int ageInSamples_ = 0;

  int noteNumber_ = 0;
  float velocity_ = 0.0f;
  double sampleRate_ = 44100.0;

  Patch patch_;

  // ymfm voice instance (to be instantiated in M1)
  // void* ymfmVoice_;

  double elapsedTime_ = 0.0;
};
