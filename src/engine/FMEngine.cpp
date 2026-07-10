#include "FMEngine.h"
#include "OPZChip.h"
#include <limits>

FMEngine::FMEngine() : chip_(std::make_unique<OPZChip>()) {
  for (int ch = 0; ch < kPolyphony; ++ch) voices_[ch].channel = ch;
}

FMEngine::~FMEngine() = default;

void FMEngine::prepare(double sampleRate, int blockSize) {
  sampleRate_ = sampleRate;
  blockSize_ = blockSize;
  chip_->prepare(sampleRate);
  for (auto& v : voices_) v.reset();
  patchDirty_ = true;
}

void FMEngine::release() {
  for (auto& v : voices_) v.reset();
}

void FMEngine::reprogramAllChannels() {
  for (int ch = 0; ch < kPolyphony; ++ch)
    chip_->programChannel(ch, currentPatch_, dx21Mask_);
  patchDirty_ = false;
}

int FMEngine::allocateChannel() {
  // 1) a fully free channel
  for (int ch = 0; ch < kPolyphony; ++ch)
    if (!voices_[ch].active && !voices_[ch].releasing) return ch;
  // 2) a channel in release tail
  for (int ch = 0; ch < kPolyphony; ++ch)
    if (voices_[ch].releasing) return ch;
  // 3) steal the oldest sounding channel
  int oldest = 0;
  uint64_t oldestTick = std::numeric_limits<uint64_t>::max();
  for (int ch = 0; ch < kPolyphony; ++ch)
    if (voices_[ch].startTick < oldestTick) { oldestTick = voices_[ch].startTick; oldest = ch; }
  return oldest;
}

void FMEngine::handleNoteOn(int note, float velocity) {
  const int ch = allocateChannel();
  Voice& v = voices_[ch];
  v.note = note;
  v.velocity = velocity;
  v.active = true;
  v.releasing = false;
  v.startTick = ++tick_;
  chip_->noteOn(ch, note, velocity);
}

void FMEngine::handleNoteOff(int note) {
  for (int ch = 0; ch < kPolyphony; ++ch) {
    Voice& v = voices_[ch];
    if (v.active && v.note == note) {
      chip_->noteOff(ch);
      v.active = false;
      v.releasing = true;   // reclaimable; envelope tail still sounds
      return;
    }
  }
}

void FMEngine::processBlock(float* outL, float* outR, int numSamples, const juce::MidiBuffer& midi) {
  if (patchDirty_) reprogramAllChannels();

  int pos = 0;
  for (const auto meta : midi) {
    const int evPos = juce::jlimit(0, numSamples, meta.samplePosition);
    if (evPos > pos) { chip_->render(outL + pos, outR + pos, evPos - pos); pos = evPos; }

    const auto msg = meta.getMessage();
    if (msg.isNoteOn())
      handleNoteOn(msg.getNoteNumber(), msg.getFloatVelocity());
    else if (msg.isNoteOff())
      handleNoteOff(msg.getNoteNumber());
    // TODO(M1+): pitch bend, sustain pedal, mod wheel, NRPN (M4).
  }
  if (pos < numSamples) chip_->render(outL + pos, outR + pos, numSamples - pos);
}

void FMEngine::setPatch(const Patch& patch) {
  currentPatch_ = patch;
  patchDirty_ = true;
}

Patch FMEngine::getPatch() const { return currentPatch_; }

void FMEngine::setParameter(int /*paramIndex*/, float /*value*/) {
  // TODO(M1): decode paramIndex -> currentPatch_ field, set patchDirty_.
  patchDirty_ = true;
}

void FMEngine::setDX21Mask(bool on) {
  dx21Mask_ = on;
  patchDirty_ = true;
}

void FMEngine::setEngineMode(EngineMode mode) {
  mode_ = mode;
  // TODO(M1): Modern = multiple OPZ chips (higher poly) + oversampled resampler.
}
