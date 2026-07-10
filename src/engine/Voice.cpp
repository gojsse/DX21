#include "Voice.h"

Voice::Voice() = default;

void Voice::prepare(double sampleRate) {
  sampleRate_ = sampleRate;
  // TODO: initialize ymfm voice instance here
}

void Voice::noteOn(int noteNumber, float velocity) {
  noteNumber_ = noteNumber;
  velocity_ = velocity;
  active_ = true;
  ageInSamples_ = 0;
  // TODO: call ymfm_voice->note_on()
}

void Voice::noteOff(float velocity) {
  // TODO: call ymfm_voice->note_off()
  // active_ = false;  // controlled by envelope release
}

void Voice::pitchBend(float semitones) {
  // TODO: update frequency offset
}

void Voice::setParameter(int operatorIndex, int paramIndex, float value) {
  // TODO: update patch.operators[operatorIndex] and apply to ymfm voice
}

void Voice::renderBlock(float* outL, float* outR, int numSamples) {
  // TODO: call ymfm_voice->generate() and mix to output
  // For now, output silence
  std::fill_n(outL, numSamples, 0.0f);
  std::fill_n(outR, numSamples, 0.0f);
  ageInSamples_ += numSamples;
}

void Voice::setPatch(const Patch& patch) {
  patch_ = patch;
  // TODO: reconfigure ymfm voice operators
}
