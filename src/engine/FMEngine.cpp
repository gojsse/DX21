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
  chip_->programLFO(currentPatch_);  // chip-global LFO (once)
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
      if (sustainOn_) {
        v.heldBySustain = true;  // keep sounding until the pedal lifts
      } else {
        chip_->noteOff(ch);
        v.active = false;
        v.releasing = true;      // reclaimable; envelope tail still sounds
      }
      return;
    }
  }
}

void FMEngine::releaseSustained() {
  for (int ch = 0; ch < kPolyphony; ++ch) {
    Voice& v = voices_[ch];
    if (v.heldBySustain) {
      chip_->noteOff(ch);
      v.active = false;
      v.releasing = true;
      v.heldBySustain = false;
    }
  }
}

void FMEngine::updateExpression() {
  chip_->setExpression((ccVolume_ / 127.0f) * (ccExpression_ / 127.0f));
}

void FMEngine::handleController(int cc, int value) {
  switch (cc) {
    case 64:  // sustain pedal
      if (value >= 64) { sustainOn_ = true; }
      else { sustainOn_ = false; releaseSustained(); }
      break;
    case 1:   chip_->setModWheel(value / 127.0f); break;         // mod wheel -> vibrato
    case 7:   ccVolume_ = value;     updateExpression(); break;  // channel volume
    case 11:  ccExpression_ = value; updateExpression(); break;  // expression
    case 123: // all notes off
      for (int ch = 0; ch < kPolyphony; ++ch) { chip_->noteOff(ch); voices_[ch].reset(); voices_[ch].channel = ch; }
      break;
    default: break;  // TODO(M4): breath (amplitude + EG bias), NRPN
  }
}

void FMEngine::handlePitchBend(int value14) {
  const float norm = (value14 - 8192) / 8192.0f;  // -1..~1
  chip_->setPitchBendSemitones(norm * static_cast<float>(currentPatch_.pb_range));
}

void FMEngine::processBlock(float* outL, float* outR, int numSamples, const juce::MidiBuffer& midi) {
  if (patchDirty_) { reprogramAllChannels(); updateExpression(); }

  int pos = 0;
  for (const auto meta : midi) {
    const int evPos = juce::jlimit(0, numSamples, meta.samplePosition);
    if (evPos > pos) { chip_->render(outL + pos, outR + pos, evPos - pos); pos = evPos; }

    const auto msg = meta.getMessage();
    if (msg.isNoteOn())
      handleNoteOn(msg.getNoteNumber(), msg.getFloatVelocity());
    else if (msg.isNoteOff())
      handleNoteOff(msg.getNoteNumber());
    else if (msg.isPitchWheel())
      handlePitchBend(msg.getPitchWheelValue());
    else if (msg.isController())
      handleController(msg.getControllerNumber(), msg.getControllerValue());
    // TODO(M4): mod wheel/breath (LFO), NRPN.
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
