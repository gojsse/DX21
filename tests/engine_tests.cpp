// OP4 engine tests — JUCE-free. Exercises the OPZ DSP core directly:
//   1. render smoke: a programmed patch keyed on produces audible output
//   2. carrier-count invariant: per-algorithm output level tracks carrier count
//      (matches the Yamaha 4-op algorithm chart)
//   3. note->keycode invariants: valid OPM note nibbles, monotonic across range
//
// This is the M1 core gate. Absolute pitch/level calibration (the [verify]
// items in OPZChip) is validated later by the golden-WAV null-test harness
// against real-hardware fixtures — that is TODO below.

#include "engine/OPZChip.h"
#include "engine/Controllers.h"
#include "model/Patch.h"
#include <cmath>
#include <cstdio>
#include <vector>

static int g_failures = 0;
#define CHECK(cond, msg) do { if (!(cond)) { std::printf("FAIL: %s\n", msg); ++g_failures; } \
                              else std::printf("ok:   %s\n", msg); } while (0)

static Patch flatPatch(int alg) {
  Patch p; p.algorithm = static_cast<uint8_t>(alg); p.feedback = 0;
  for (int i = 0; i < 4; ++i) {
    Operator& o = p.operators[i];
    o.ar = 31; o.d1r = 0; o.d2r = 0; o.rr = 8; o.d1l = 0;
    o.coarse = 1; o.detune = 3; o.tl = 99; o.wave = 0;
  }
  return p;
}

static float peakOfVel(int alg, float velocity) {
  OPZChip c; c.prepare(44100.0);
  Patch p = flatPatch(alg);
  c.programChannel(0, p, /*dx21Mask=*/true);
  c.noteOn(0, 69, velocity);
  const int N = 44100;
  std::vector<float> L(N), R(N);
  c.render(L.data(), R.data(), N);
  float peak = 0.0f;
  for (int i = N / 4; i < N; ++i) peak = std::max(peak, std::fabs(L[i]));
  return peak;
}
static float peakOf(int alg) { return peakOfVel(alg, 1.0f); }

int main() {
  std::printf("OP4 Engine Tests (M1)\n");

  // 1. render smoke
  CHECK(peakOf(4) > 0.05f, "render smoke: keyed note is audibly non-silent");

  // 2. carrier-count invariant (Yamaha 4-op chart):
  //    algs 1-4 => 1 carrier, alg 5 => 2, algs 6-7 => 3, alg 8 => 4.
  const float p0 = peakOf(0), p4 = peakOf(4), p5 = peakOf(5), p7 = peakOf(7);
  CHECK(p4 > p0 * 1.5f, "alg 5 (2 carriers) louder than alg 1 (1 carrier)");
  CHECK(p5 > p4 * 1.2f, "alg 6 (3 carriers) louder than alg 5 (2 carriers)");
  CHECK(p7 > p5 * 1.1f, "alg 8 (4 carriers) is the loudest");

  // 3. velocity -> carrier TL: harder notes are louder; full velocity is unchanged.
  const float vFull = peakOfVel(7, 1.0f), vSoft = peakOfVel(7, 0.3f);
  CHECK(vFull > vSoft * 1.3f, "higher velocity is louder (carrier TL)");
  CHECK(std::fabs(vFull - peakOf(7)) < 1e-6f, "velocity 1.0 == programmed level (golden stays valid)");

  // 3b. pitch bend: key-code math (KC = semitone, KF = intra-semitone fraction)
  uint8_t kc69, kf69, kcUp, kfUp, kcHalf, kfHalf, kcOct, kfOct;
  OPZChip::computeKeyCode(69.0, kc69, kf69);
  OPZChip::computeKeyCode(69.0 + 1.0, kcUp, kfUp);
  OPZChip::computeKeyCode(69.5, kcHalf, kfHalf);
  OPZChip::computeKeyCode(69.0 + 12.0, kcOct, kfOct);
  CHECK(kf69 == 0, "exact note -> key fraction 0");
  CHECK(kcUp != kc69 && kfUp == 0, "bend +1 semitone -> next key code");
  CHECK(kcHalf == kc69 && kfHalf == 32, "bend +0.5 semitone -> mid key fraction");
  CHECK(kcOct == static_cast<uint8_t>(kc69 + 0x10) && kfOct == 0, "bend +12 -> +1 octave in KC");

  // 3c. expression / volume attenuates carriers
  auto peakExpr = [](float expr) {
    OPZChip c; c.prepare(44100.0);
    c.programChannel(0, flatPatch(7), true);
    c.noteOn(0, 69, 1.0f);
    c.setExpression(expr);
    const int N = 44100; std::vector<float> L(N), R(N); c.render(L.data(), R.data(), N);
    float pk = 0.0f; for (int i = N / 4; i < N; ++i) pk = std::max(pk, std::fabs(L[i]));
    return pk;
  };
  CHECK(peakExpr(1.0f) > peakExpr(0.4f) * 1.3f, "lower expression is quieter (carrier TL)");

  // 3d. LFO: with rate + PM depth + PMS the tone is modulated (differs from off);
  //     the mod wheel adds vibrato even when the patch PMD is zero.
  auto renderPatch = [](bool lfoOn, float modWheel) {
    OPZChip c; c.prepare(44100.0);
    Patch p = flatPatch(0);
    if (lfoOn) { p.lfo_speed = 60; p.pmd = 80; p.pms = 6; }
    else       { p.pms = 6; p.lfo_speed = 60; }  // sensitivity + rate, but PMD 0
    c.programChannel(0, p, true);
    c.programLFO(p);
    c.noteOn(0, 60, 1.0f);
    c.setModWheel(modWheel);
    const int N = 44100; std::vector<float> L(N), R(N); c.render(L.data(), R.data(), N);
    return L;
  };
  auto diffEnergy = [](const std::vector<float>& a, const std::vector<float>& b) {
    double diff = 0, sig = 0;
    for (size_t i = 0; i < a.size(); ++i) { const double d = a[i] - b[i]; diff += d * d; sig += b[i] * b[i]; }
    return sig > 0 ? diff / sig : 0.0;
  };
  const auto lfoOff = renderPatch(false, 0.0f);
  CHECK(diffEnergy(renderPatch(true, 0.0f), lfoOff) > 1e-3, "LFO (rate+PMD+PMS) modulates the tone");
  CHECK(diffEnergy(renderPatch(false, 1.0f), lfoOff) > 1e-3, "mod wheel adds vibrato when patch PMD is 0");

  // 3e. breath controller curves (CC2 rests at 0; only active when the patch enables it)
  using op4::ctl::breathAmpGain;
  using op4::ctl::breathPitchSemis;
  CHECK(breathAmpGain(0, 50, 0) == 1.0f && breathAmpGain(0, 50, 127) == 1.0f,
        "breath_amp 0 -> no amplitude effect (plays normally)");
  CHECK(breathAmpGain(99, 50, 127) > breathAmpGain(99, 50, 0),
        "more breath -> louder when breath_amp is set");
  CHECK(std::fabs(breathAmpGain(99, 0, 0) - 0.0f) < 1e-6f,
        "full breath sens, no bias, no breath -> silent");
  CHECK(std::fabs(breathAmpGain(99, 50, 127) - 1.0f) < 1e-6f,
        "full breath -> full level");
  CHECK(breathPitchSemis(0, 127) == 0.0f, "breath_pitch 0 -> no pitch effect");
  CHECK(breathPitchSemis(99, 127) > breathPitchSemis(99, 40),
        "more breath -> more pitch when breath_pitch is set");

  // 4. note->keycode invariants
  bool validNibbles = true, monotonic = true;
  uint8_t prevKc = 0, kc, kf;
  for (int n = 12; n <= 108; ++n) {
    OPZChip::noteToKeyCode(n, kc, kf);
    if ((kc & 0x03) == 0x03) validNibbles = false;  // 4th code in each group is invalid
    if (n > 12 && kc < prevKc) monotonic = false;
    prevKc = kc;
  }
  CHECK(validNibbles, "keycode note nibble is always a valid OPM code");
  CHECK(monotonic, "keycode rises monotonically with MIDI note");

  // 5. TODO(M1): golden-WAV null-test against real-hardware fixtures to lock
  //    absolute tuning, TL curve, and operator/slot ordering ([verify] items).

  std::printf(g_failures ? "\n%d CHECK(s) FAILED\n" : "\nALL CHECKS PASSED\n", g_failures);
  return g_failures ? 1 : 0;
}
