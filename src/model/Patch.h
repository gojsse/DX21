#pragma once
#include <array>
#include <cstdint>
#include <string>

// Neutral patch model — the hub between codec, engine, UI, generator
struct Operator {
  uint8_t ar = 0, d1r = 0, d2r = 0, rr = 0;  // envelope rates
  uint8_t d1l = 0;                            // decay 1 level
  uint8_t tl = 0;                              // total level (output)
  uint8_t rs = 0, ks = 0;                      // rate/key scaling
  uint8_t eg_shift = 0;                        // TX81Z only
  uint8_t am = 0;                              // amplitude mod enable
  uint8_t kvs = 0;                             // key velocity sensitivity
  uint8_t coarse = 0, fine = 0;                // frequency ratio / fine
  uint8_t detune = 3;                          // detune (-63 to +63, center=3)
  uint8_t fixed = 0;                           // TX81Z: fixed frequency flag
  uint8_t fixed_freq_range = 0;                // TX81Z: range 0-7
  uint8_t wave = 0;                            // TX81Z: waveform 0-7
  uint8_t egl = 0, eg2 = 0, eg3 = 0;           // pitch EG levels (DX21 mode)

  bool operator==(const Operator&) const = default;
};

struct Patch {
  std::string name;
  uint8_t algorithm = 0;           // 0-7
  uint8_t feedback = 0;             // 0-7 (OP4 self-feedback)
  std::array<Operator, 4> operators;

  // LFO
  uint8_t lfo_speed = 0, lfo_delay = 0;
  uint8_t pmd = 0, amd = 0;                // pitch/amplitude mod depth
  uint8_t lfo_sync = 0;
  uint8_t lfo_wave = 0;                    // 0=saw, 1=sqr, 2=tri, 3=S&H
  uint8_t pms = 0, ams = 0;                // pitch/amp mod sensitivity

  // Pitch EG
  uint8_t peg_r1 = 0, peg_r2 = 0, peg_r3 = 0;
  uint8_t peg_l0 = 0, peg_l1 = 0, peg_l2 = 0, peg_l3 = 0;

  // Function
  uint8_t transpose = 24;                  // 0-48 (C0-C4, center=C3)
  uint8_t poly_mono = 0;                   // 0=poly, 1=mono
  uint8_t pb_range = 2;                    // pitch bend range in semitones
  uint8_t portamento_mode = 0;
  uint8_t portamento_time = 0;
  uint8_t foot_mod_pitch = 0, foot_mod_amp = 0;
  uint8_t sustain = 0, portamento_on = 0;

  // Mod wheel
  uint8_t mw_pitch = 0, mw_amp = 0;
  uint8_t breath_pitch = 0, breath_amp = 0, breath_bias = 50;
  uint8_t breath_eg = 0;

  // Chorus (DX21/TX81Z, minimal support)
  uint8_t chorus = 0;

  // TX81Z extensions (ACED)
  uint8_t reverb = 0;
  uint8_t foot_ctrl_pitch = 0, foot_ctrl_amp = 0;

  // Utility
  bool operator==(const Patch& other) const = default;
};
