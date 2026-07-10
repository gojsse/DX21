// PatchJson tests — the WebView bridge wire format. Verifies:
//   1. full round-trip identity (Patch -> JSON -> Patch)
//   2. overlay() applies only present keys (partial messages are safe)
// Links juce_core (for var/JSON); no plugin/audio deps.

#include "bridge/PatchJson.h"
#include "model/Patch.h"
#include <cstdio>

static int g_fail = 0;
#define CHECK(c, m) do { if (!(c)) { std::printf("FAIL: %s\n", m); ++g_fail; } \
                         else std::printf("ok:   %s\n", m); } while (0)

static Patch varied() {
  Patch p;
  p.name = "NITE PAD";
  p.algorithm = 5; p.feedback = 6;
  for (int i = 0; i < 4; ++i) {
    Operator& o = p.operators[i];
    o.ar = 31 - i; o.d1r = 10 + i; o.d2r = i; o.rr = 8 + i; o.d1l = 2 + i;
    o.tl = 40 + i * 10; o.rs = i % 4; o.ks = i % 3; o.coarse = i + 1; o.fine = i;
    o.detune = 3 + i; o.wave = i; o.eg_shift = i % 4; o.am = i & 1; o.kvs = i;
    o.fixed = 0; o.fixed_freq_range = i; o.egl = i; o.eg2 = i * 2; o.eg3 = i * 3;
  }
  p.lfo_speed = 42; p.lfo_wave = 2; p.lfo_sync = 1; p.pmd = 12; p.amd = 7; p.pms = 3; p.ams = 1;
  p.peg_r1 = 5; p.peg_l0 = 50; p.transpose = 12; p.poly_mono = 1; p.pb_range = 12;
  p.reverb = 4; p.breath_bias = 50;
  return p;
}

int main() {
  std::printf("PatchJson Tests\n");

  const Patch original = varied();
  const juce::String json = op4::patchjson::encode(original);
  const Patch back = op4::patchjson::decode(json);
  CHECK(back == original, "full round-trip is identity (Patch -> JSON -> Patch)");

  // A partial message must not disturb other fields.
  Patch base = varied();
  const Patch before = base;
  op4::patchjson::overlay(base, juce::JSON::parse(R"({"algorithm": 3})"));
  CHECK(base.algorithm == 3, "overlay applies the present key");
  base.algorithm = before.algorithm;  // undo the one intended change
  CHECK(base == before, "overlay left every other field untouched");

  // A partial operator message updates one op field only.
  Patch op = varied();
  const uint8_t oldTl1 = op.operators[1].tl;
  op4::patchjson::overlay(op, juce::JSON::parse(R"({"operators":[{},{"tl": 7}]})"));
  CHECK(op.operators[1].tl == 7, "overlay updates a single operator field");
  CHECK(op.operators[1].tl != oldTl1 && op.operators[0].tl == varied().operators[0].tl,
        "overlay leaves sibling operators untouched");

  std::printf(g_fail ? "\n%d CHECK(s) FAILED\n" : "\nALL CHECKS PASSED\n", g_fail);
  return g_fail ? 1 : 0;
}
