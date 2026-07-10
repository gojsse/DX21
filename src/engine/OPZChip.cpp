#include "OPZChip.h"
#include <algorithm>
#include <cmath>

OPZChip::OPZChip() : m_chip(*this) {
  m_chipRate = m_chip.sample_rate(kInputClock);
}

void OPZChip::prepare(double hostSampleRate) {
  m_hostRate = hostSampleRate;
  m_chipRate = m_chip.sample_rate(kInputClock);
  reset();
}

void OPZChip::reset() {
  m_chip.reset();
  m_ch20base.fill(0x80);  // output-left enabled, key-on clear
  m_phase = 0.0;
  m_prevL = m_prevR = m_curL = m_curR = 0.0f;
  // prime the resampler with two chip samples
  pullChipSample();
  pullChipSample();
}

void OPZChip::writeReg(uint8_t addr, uint8_t data) {
  m_chip.write_address(addr);
  m_chip.write_data(data);
}

// --- calibration tables --------------------------------------------------

// DX OP1..OP4 -> chip operator-slot 0..3.
// [verify] Algorithm-critical. Identity is the placeholder; the OPM/OPZ family
// physically interleaves slots as {0,2,1,3}, so once the null-test harness is
// up this is the single line to correct if algorithms sound scrambled.
int OPZChip::slotForOp(int op) {
  static const int kSlotForOp[4] = {0, 1, 2, 3};  // [verify] try {0,2,1,3}
  return kSlotForOp[op & 3];
}

// DX coarse-ratio index (0..31) -> chip MULTIPLE (0..15, 0 => 0.5x).
// [verify] First-pass clamp; real DX ratio table has fractional steps handled
// via the OW/FINE low nibble.
uint8_t OPZChip::coarseToMul(uint8_t coarse) {
  return static_cast<uint8_t>(std::min<int>(coarse, 15));
}

// DX output level (0..99, 99=loud) -> chip TOTAL LEVEL (0..127, 0=loud).
// [verify] Linear approximation; the real DX curve is non-linear near the top.
uint8_t OPZChip::levelToTL(uint8_t level) {
  int l = std::clamp<int>(level, 0, 99);
  return static_cast<uint8_t>(std::clamp((99 - l) * 127 / 99, 0, 127));
}

// Carrier operators (bit i = op i) per algorithm 0..7. From the verified 4-op
// chart (matches the measured carrier counts: 1,1,1,1,2,3,3,4). [verify] with
// the op->slot ordering.
uint8_t OPZChip::carrierMask(int algorithm) {
  static const uint8_t kCarrier[8] = {
    0b0001, 0b0001, 0b0001, 0b0001,  // one carrier (OP1)
    0b0101,                          // OP1, OP3
    0b0111, 0b0111,                  // OP1, OP2, OP3
    0b1111,                          // all four
  };
  return kCarrier[algorithm & 7];
}

// MIDI note -> OPM/OPZ key code (reg 0x28) + key fraction (reg 0x30 bits 7-2).
// KC layout: bits 6-4 octave, bits 3-0 note. Valid note nibbles skip every 4th
// value: semitone s -> (s/3)*4 + (s%3). [verify] absolute octave reference.
void OPZChip::noteToKeyCode(int midiNote, uint8_t& kc, uint8_t& kf) {
  // The chip's key-code octave field is 3 bits (0-7), so it spans 8 octaves.
  // Clamp the note into that representable window; out-of-range notes saturate
  // flat rather than wrapping. [verify] absolute octave reference against A440.
  midiNote = std::clamp(midiNote, 12, 107);
  int semitone = midiNote % 12;                 // 0 = C
  int note = (semitone / 3) * 4 + (semitone % 3);
  int octave = midiNote / 12 - 1;               // 0..7 within [12,107]
  kc = static_cast<uint8_t>((octave << 4) | note);
  kf = 0;  // exact equal temperament for now; microtuning/bend feed this later
}

// --- programming ---------------------------------------------------------

void OPZChip::programChannel(int ch, const Patch& p, bool dx21Mask) {
  ch &= 7;
  const uint8_t alg = static_cast<uint8_t>(p.algorithm & 7);
  const uint8_t fb  = static_cast<uint8_t>(p.feedback & 7);
  m_ch20base[ch] = static_cast<uint8_t>(0x80 | (fb << 3) | alg);

  // channel-level: output/feedback/algorithm (key-on clear during programming)
  writeReg(0x20 + ch, m_ch20base[ch]);
  // PMS/AMS (LFO sensitivity)
  writeReg(0x38 + ch, static_cast<uint8_t>(((p.pms & 7) << 4) | (p.ams & 3)));

  for (int op = 0; op < 4; ++op) {
    const Operator& o = p.operators[op];
    const int slot = slotForOp(op);
    const uint8_t base = static_cast<uint8_t>((slot << 3) | ch);

    const uint8_t mul = coarseToMul(o.coarse);
    const uint8_t dt1 = static_cast<uint8_t>(o.detune & 7);
    writeReg(0x40 + base, static_cast<uint8_t>((dt1 << 4) | mul));  // bit7=0: DT1/MUL

    // OW/FINE (banked via bit7=1). DX21 mask forces sine (waveform 0).
    const uint8_t wave = dx21Mask ? 0 : static_cast<uint8_t>(o.wave & 7);
    writeReg(0x40 + base, static_cast<uint8_t>(0x80 | (wave << 4) | (o.fine & 0x0f)));

    const uint8_t tl = levelToTL(o.tl);
    m_baseTL[ch][op] = tl;
    writeReg(0x60 + base, tl);                                     // TL

    const uint8_t fix = dx21Mask ? 0 : static_cast<uint8_t>(o.fixed & 1);
    writeReg(0x80 + base,                                          // KRS/FIX/AR
             static_cast<uint8_t>(((o.rs & 3) << 6) | (fix << 5) | (o.ar & 0x1f)));
    writeReg(0xA0 + base,                                          // AM/D1R
             static_cast<uint8_t>(((o.am & 1) << 7) | (o.d1r & 0x1f)));
    writeReg(0xC0 + base,                                          // DT2/D2R (bit5=0)
             static_cast<uint8_t>(o.d2r & 0x1f));
    if (!dx21Mask) {                                               // EGS/REV (banked, bit5=1)
      writeReg(0xC0 + base,
               static_cast<uint8_t>(0x20 | ((o.eg_shift & 3) << 6) | (p.reverb & 7)));
    }
    writeReg(0xE0 + base,                                          // D1L/RR
             static_cast<uint8_t>(((o.d1l & 0x0f) << 4) | (o.rr & 0x0f)));
  }
}

void OPZChip::noteOn(int ch, int midiNote, float velocity) {
  ch &= 7;
  uint8_t kc, kf;
  noteToKeyCode(midiNote, kc, kf);
  writeReg(0x28 + ch, kc);
  writeReg(0x30 + ch, static_cast<uint8_t>((kf << 2) | 0x01));  // KF + output-right on

  // Velocity -> carrier TL: attenuate carriers below their programmed level as
  // velocity drops (0 dB at velocity 1.0). Modulator level (timbre) is left
  // alone for now. TODO: weight by per-op KVS.
  const uint8_t mask = carrierMask(m_ch20base[ch] & 0x07);
  const int atten = static_cast<int>(std::lround((1.0f - std::clamp(velocity, 0.0f, 1.0f)) * kVelocityDepthTL));
  for (int op = 0; op < 4; ++op) {
    if (mask & (1 << op)) {
      const uint8_t base = static_cast<uint8_t>((slotForOp(op) << 3) | ch);
      writeReg(0x60 + base, static_cast<uint8_t>(std::clamp(m_baseTL[ch][op] + atten, 0, 127)));
    }
  }

  writeReg(0x08, static_cast<uint8_t>(ch));                     // select channel for key event
  writeReg(0x20 + ch, static_cast<uint8_t>(m_ch20base[ch] | 0x40));  // key on (bit6)
}

void OPZChip::noteOff(int ch) {
  ch &= 7;
  writeReg(0x08, static_cast<uint8_t>(ch));
  writeReg(0x20 + ch, static_cast<uint8_t>(m_ch20base[ch] & ~0x40));  // key off
}

// --- rendering -----------------------------------------------------------

void OPZChip::pullChipSample() {
  ymfm::ym2414::output_data out;
  m_chip.generate(&out, 1);
  m_prevL = m_curL;
  m_prevR = m_curR;
  m_curL = static_cast<float>(out.data[0]) / 32768.0f;
  m_curR = static_cast<float>(out.data[1]) / 32768.0f;
}

void OPZChip::renderNative(int16_t* out, int numFrames) {
  for (int i = 0; i < numFrames; ++i) {
    ymfm::ym2414::output_data o;
    m_chip.generate(&o, 1);
    out[2 * i]     = static_cast<int16_t>(std::clamp<int32_t>(o.data[0], -32768, 32767));
    out[2 * i + 1] = static_cast<int16_t>(std::clamp<int32_t>(o.data[1], -32768, 32767));
  }
}

void OPZChip::render(float* outL, float* outR, int numSamples) {
  const double step = static_cast<double>(m_chipRate) / m_hostRate;  // chip samples / host sample
  for (int i = 0; i < numSamples; ++i) {
    m_phase += step;
    while (m_phase >= 1.0) {
      pullChipSample();
      m_phase -= 1.0;
    }
    const float t = static_cast<float>(m_phase);  // 0..1 between prev and cur
    outL[i] = m_prevL + (m_curL - m_prevL) * t;
    outR[i] = m_prevR + (m_curR - m_prevR) * t;
  }
}
