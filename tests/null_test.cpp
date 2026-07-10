// OP4 null-test / golden-WAV harness — JUCE-free.
//
// Two jobs:
//   1. GOLDEN REGRESSION (CI): render a fixed phrase at the chip's native rate
//      (pure-integer ymfm, bit-deterministic across platforms) and null it
//      against a committed golden WAV. Guards against unintended DSP changes.
//   2. HARDWARE NULL-TEST (local, private): `compare ours.wav hardware.wav`
//      reports the null depth in dB, for locking the OPZChip [verify] items
//      (tuning, TL curve, ratio table, slot order) against a real TX81Z capture.
//      Hardware WAVs are private fixtures — never committed (legal/IP).
//
// Usage:
//   null_test golden <golden.wav> [--update] [--threshold dB]
//   null_test render <out.wav>
//   null_test compare <a.wav> <b.wav> [--threshold dB]

#include "engine/OPZChip.h"
#include "model/Patch.h"
#include "wav.h"
#include <cmath>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

// The canonical phrase's patch: a fixed 2-carrier voice (nothing Yamaha).
static Patch phrasePatch() {
  Patch p;
  p.name = "NULLTEST";
  p.algorithm = 4;  // two modulator->carrier pairs
  p.feedback = 0;
  for (int i = 0; i < 4; ++i) {
    Operator& o = p.operators[i];
    o.ar = 31; o.d1r = 6; o.d2r = 0; o.rr = 8; o.d1l = 2;
    o.coarse = 1; o.detune = 3; o.wave = 0;
  }
  p.operators[0].tl = 99; p.operators[2].tl = 99;  // carriers
  p.operators[1].tl = 78; p.operators[3].tl = 78;  // modulators
  return p;
}

// A deterministic phrase: four notes, each keyed on then released.
static void renderPhrase(std::vector<int16_t>& out, int& sampleRate) {
  constexpr int kOnFrames = 4000, kReleaseFrames = 2000;
  const int notes[] = {48, 55, 60, 67};  // C3 G3 C4 G4

  OPZChip chip;
  sampleRate = static_cast<int>(chip.chipSampleRate());
  chip.prepare(static_cast<double>(sampleRate));
  chip.programChannel(0, phrasePatch(), /*dx21Mask=*/true);

  out.clear();
  for (int note : notes) {
    chip.noteOn(0, note, 1.0f);
    size_t base = out.size();
    out.resize(base + static_cast<size_t>(kOnFrames) * 2);
    chip.renderNative(out.data() + base, kOnFrames);

    chip.noteOff(0);
    base = out.size();
    out.resize(base + static_cast<size_t>(kReleaseFrames) * 2);
    chip.renderNative(out.data() + base, kReleaseFrames);
  }
}

static void stats(const std::vector<int16_t>& s, double& rms, double& peak) {
  double sumSq = 0.0; int pk = 0;
  for (int16_t v : s) { sumSq += double(v) * v; pk = std::max(pk, std::abs(int(v))); }
  rms = s.empty() ? 0.0 : std::sqrt(sumSq / s.size()) / 32768.0;
  peak = pk / 32768.0;
}

// Null depth in dB: 10*log10(diffEnergy / signalEnergy). Lower = better null.
static double nullDepthDb(const std::vector<int16_t>& a, const std::vector<int16_t>& b) {
  const size_t n = std::min(a.size(), b.size());
  double diff = 0.0, sig = 0.0;
  for (size_t i = 0; i < n; ++i) {
    const double d = double(a[i]) - double(b[i]);
    diff += d * d;
    sig += double(a[i]) * double(a[i]);
  }
  if (sig <= 0.0) return 0.0;
  if (diff <= 0.0) return -300.0;  // perfect null
  return 10.0 * std::log10(diff / sig);
}

static double argThreshold(int argc, char** argv, double def) {
  for (int i = 0; i < argc - 1; ++i)
    if (std::strcmp(argv[i], "--threshold") == 0) return std::atof(argv[i + 1]);
  return def;
}
static bool hasFlag(int argc, char** argv, const char* f) {
  for (int i = 0; i < argc; ++i) if (std::strcmp(argv[i], f) == 0) return true;
  return false;
}

int main(int argc, char** argv) {
  const std::string mode = argc > 1 ? argv[1] : "golden";

  if (mode == "render") {
    if (argc < 3) { std::printf("usage: null_test render <out.wav>\n"); return 2; }
    std::vector<int16_t> s; int sr = 0;
    renderPhrase(s, sr);
    if (!op4::wav::writePCM16(argv[2], s, sr, 2)) { std::printf("FAIL: write %s\n", argv[2]); return 1; }
    double rms, pk; stats(s, rms, pk);
    std::printf("wrote %s (%zu frames @ %d Hz)  rms=%.4f peak=%.4f\n",
                argv[2], s.size() / 2, sr, rms, pk);
    return 0;
  }

  if (mode == "compare") {
    if (argc < 4) { std::printf("usage: null_test compare <a.wav> <b.wav> [--threshold dB]\n"); return 2; }
    std::vector<int16_t> a, b; int sra = 0, srb = 0, ca = 0, cb = 0;
    if (!op4::wav::readPCM16(argv[2], a, sra, ca)) { std::printf("FAIL: read %s\n", argv[2]); return 1; }
    if (!op4::wav::readPCM16(argv[3], b, srb, cb)) { std::printf("FAIL: read %s\n", argv[3]); return 1; }
    if (sra != srb) std::printf("WARN: sample-rate mismatch (%d vs %d) — align before trusting the null\n", sra, srb);
    const double nd = nullDepthDb(a, b), thr = argThreshold(argc, argv, -60.0);
    std::printf("null depth: %.1f dB (threshold %.1f dB)\n", nd, thr);
    const bool pass = nd <= thr;
    std::printf("%s\n", pass ? "PASS" : "FAIL");
    return pass ? 0 : 1;
  }

  // default: golden regression
  if (argc < 3) { std::printf("usage: null_test golden <golden.wav> [--update] [--threshold dB]\n"); return 2; }
  const std::string golden = argv[2];
  std::vector<int16_t> s; int sr = 0;
  renderPhrase(s, sr);
  double rms, pk; stats(s, rms, pk);
  std::printf("OP4 null-test: phrase %zu frames @ %d Hz  rms=%.4f peak=%.4f\n", s.size() / 2, sr, rms, pk);

  std::vector<int16_t> ref; int rsr = 0, rc = 0;
  const bool haveGolden = op4::wav::readPCM16(golden, ref, rsr, rc);

  if (hasFlag(argc, argv, "--update") || !haveGolden) {
    if (!op4::wav::writePCM16(golden, s, sr, 2)) { std::printf("FAIL: write golden %s\n", golden.c_str()); return 1; }
    std::printf("%s golden %s\n", haveGolden ? "updated" : "wrote", golden.c_str());
    return 0;
  }

  if (ref.size() != s.size())
    std::printf("WARN: golden length %zu != render %zu\n", ref.size(), s.size());
  const double nd = nullDepthDb(s, ref), thr = argThreshold(argc, argv, -60.0);
  std::printf("null vs golden: %.1f dB (threshold %.1f dB)\n", nd, thr);
  const bool pass = nd <= thr;
  std::printf("%s\n", pass ? "PASS: engine matches golden" : "FAIL: engine diverged from golden (run --update if intended)");
  return pass ? 0 : 1;
}
