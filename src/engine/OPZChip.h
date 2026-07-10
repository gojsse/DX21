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

  // Program the chip-global LFO from the patch (rate/waveform/sync + PM/AM
  // depth). Per-channel PM/AM sensitivity (PMS/AMS) is written by programChannel.
  void programLFO(const Patch& p);

  // Mod wheel 0..1 -> extra LFO pitch-mod depth (adds to the patch's PMD).
  void setModWheel(float amount01);

  // Global pitch bend in semitones (applied to every channel via KC/KF).
  void setPitchBendSemitones(float semitones);
  // Global expression/volume gain 0..1, applied as carrier attenuation
  // (combines with per-note velocity). 1.0 = programmed level.
  void setExpression(float gain01);

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
  // Fractional key code: KC (semitone) + KF (6-bit intra-semitone fraction),
  // used for pitch bend / microtuning.
  static void computeKeyCode(double midiNote, uint8_t& kc, uint8_t& kf);

  // Carrier operators (bitmask over ops 0..3) for algorithm 0..7. Derived from
  // the verified 4-op chart; matches the measured per-algorithm carrier counts.
  static uint8_t carrierMask(int algorithm);

private:
  void writeReg(uint8_t addr, uint8_t data);
  void pullChipSample();
  void writePitch(int ch);        // KC/KF from note + bend
  void writeCarrierTL(int ch);    // base TL + velocity + expression attenuation
  void writePmDepth();            // LFO PM depth = patch PMD + mod wheel

  // Mod wheel fully up adds this many steps of LFO pitch-mod depth (0..127). [verify]
  static constexpr int kModWheelDepthPMD = 64;

  // Velocity -> carrier TL: attenuation in TL steps (~0.75 dB each) at velocity
  // 0; at velocity 1.0 the carriers play at their programmed level. [verify]
  static constexpr int kVelocityDepthTL = 48;
  // Expression/volume 0 -> this much carrier attenuation (TL steps). [verify]
  static constexpr int kExpressionDepthTL = 64;

  ymfm::ym2414 m_chip;
  uint32_t m_chipRate = 0;
  double   m_hostRate = 44100.0;

  // cached reg 0x20 base per channel (output|feedback|algorithm, key-on bit clear)
  std::array<uint8_t, kChannels> m_ch20base{};

  // programmed TL per channel per op (0..3), so note-on can offset carriers by
  // velocity without re-deriving from the patch.
  std::array<std::array<uint8_t, 4>, kChannels> m_baseTL{};

  // per-channel note + performance controllers (applied via KC/KF and TL).
  std::array<int, kChannels> m_note{};         // base MIDI note per channel
  std::array<int, kChannels> m_velAtten{};     // velocity attenuation (TL steps)
  float m_bendSemitones = 0.0f;
  float m_expression = 1.0f;                    // CC7*CC11 gain 0..1
  int m_basePmd = 0;                            // patch LFO PM depth (reg units)
  int m_modWheelPmd = 0;                        // mod-wheel PM depth boost

  // linear-resampler state (spike-quality; [verify] — replace with polyphase)
  double m_phase = 0.0;
  float  m_prevL = 0.0f, m_prevR = 0.0f;
  float  m_curL = 0.0f, m_curR = 0.0f;
};
