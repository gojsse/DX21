#pragma once
#include <array>
#include <cstdint>
#include "ymfm_opz.h"
#include "../model/Patch.h"

// OPZChip — thin wrapper over ymfm's YM2414 (the OPZ, i.e. the TX81Z's actual
// FM chip). One chip = one 8-voice polyphony pool. FMEngine owns one (Vintage)
// or several (Modern) of these and maps notes to channels.
//
// This layer is intentionally JUCE-free so the DSP core can be compiled and
// bench-rendered on its own (see tests/opz_render_test.cpp).
//
// Because the OPZ is the TX81Z chip, most patch params map 1:1 to registers.
// The handful of calibration unknowns that need hardware/golden-WAV validation
// (M1 exit gate) are centralized and marked [verify]:
//   - kSlotForOp     : DX OP1..4 -> chip operator-slot order (algorithm-critical)
//   - noteToKeyCode  : absolute octave reference (which MIDI note == chip C)
//   - coarseToMul    : DX coarse-ratio index -> chip MULTIPLE
//   - levelToTL      : DX output level (0..99) -> chip TOTAL LEVEL (0..127)
class OPZChip : public ymfm::ymfm_interface {
public:
  static constexpr int kChannels = 8;
  static constexpr uint32_t kInputClock = 3579545;  // standard YM2151/2414 clock

  OPZChip();

  // Set the host sample rate; the chip runs at its native rate internally and
  // is resampled to the host rate in render().
  void prepare(double hostSampleRate);
  void reset();

  // Program a full patch onto a channel. Does not trigger a note.
  // dx21Mask: force sine waveform + disable fixed-frequency/ACED (DX21 family).
  void programChannel(int ch, const Patch& p, bool dx21Mask);

  void noteOn(int ch, int midiNote, float velocity);
  void noteOff(int ch);

  // Advance the chip and sum all 8 channels into stereo host-rate float output.
  void render(float* outL, float* outR, int numSamples);

  // Deterministic native-rate render (no resampling): the chip's integer output
  // clamped to interleaved stereo int16. Bit-identical across platforms — used
  // by the null-test / golden-WAV regression harness.
  void renderNative(int16_t* interleavedLR, int numFrames);

  uint32_t chipSampleRate() const { return m_chipRate; }

  // --- calibration helpers (see class comment; [verify] against hardware) ---
  // Public + static so the engine test can assert their invariants directly.
  static int  slotForOp(int op);          // DX OP1..4 -> chip slot 0..3
  static uint8_t coarseToMul(uint8_t coarse);
  static uint8_t levelToTL(uint8_t level);
  static void noteToKeyCode(int midiNote, uint8_t& kc, uint8_t& kf);

private:
  void writeReg(uint8_t addr, uint8_t data);
  void pullChipSample();

  ymfm::ym2414 m_chip;
  uint32_t m_chipRate = 0;
  double   m_hostRate = 44100.0;

  // cached reg 0x20 base per channel (output|feedback|algorithm, key-on bit clear)
  std::array<uint8_t, kChannels> m_ch20base{};

  // linear-resampler state (spike-quality; [verify] — replace with polyphase)
  double m_phase = 0.0;
  float  m_prevL = 0.0f, m_prevR = 0.0f;
  float  m_curL = 0.0f, m_curR = 0.0f;
};
