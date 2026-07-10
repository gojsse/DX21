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

static float peakOf(int alg) {
  OPZChip c; c.prepare(44100.0);
  Patch p = flatPatch(alg);
  c.programChannel(0, p, /*dx21Mask=*/true);
  c.noteOn(0, 69, 1.0f);
  const int N = 44100;
  std::vector<float> L(N), R(N);
  c.render(L.data(), R.data(), N);
  float peak = 0.0f;
  for (int i = N / 4; i < N; ++i) peak = std::max(peak, std::fabs(L[i]));
  return peak;
}

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

  // 3. note->keycode invariants
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

  // 4. TODO(M1): golden-WAV null-test against real-hardware fixtures to lock
  //    absolute tuning, TL curve, and operator/slot ordering ([verify] items).

  std::printf(g_failures ? "\n%d CHECK(s) FAILED\n" : "\nALL CHECKS PASSED\n", g_failures);
  return g_failures ? 1 : 0;
}
