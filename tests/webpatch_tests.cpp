// WebPatch tests — the web display model -> native DSP Patch mapping.
// Asserts the unambiguous conversions and the monotonic envelope wiring.
// Links juce_core (var/JSON); JUCE_USE_CURL=0.

#include "bridge/WebPatch.h"
#include "model/Patch.h"
#include <cstdio>

static int g_fail = 0;
#define CHECK(c, m) do { if (!(c)) { std::printf("FAIL: %s\n", m); ++g_fail; } \
                         else std::printf("ok:   %s\n", m); } while (0)

// A web-shaped patch (matches src/state/types.ts), incl. a Unicode-minus detune.
static const char* kWebJson = R"({
  "algorithm": 5,
  "feedback": 4,
  "ops": [
    {"lvl":"99","det":"+2","wave":"W2 SQR","fix":false,"ratio":"14.00",
     "ep":{"a":0.0,"d1":0.5,"s":1.0,"r":0.0}},
    {"lvl":"58","det":"−4","wave":"W3 TRI","fix":true,"ratio":"0.50",
     "ep":{"a":1.0,"d1":0.0,"s":0.0,"r":1.0}},
    {"lvl":"66","det":"+0","wave":"W1 SIN","fix":false,"ratio":"1.00",
     "ep":{"a":0.5,"d1":0.5,"s":0.5,"r":0.5}},
    {"lvl":"41","det":"+0","wave":"W1 SIN","fix":false,"ratio":"5.00",
     "ep":{"a":0.1,"d1":0.1,"s":0.1,"r":0.1}}
  ],
  "lfo": {"wave":"TRI","sync":true,
          "knobs":[{"key":"speed","val":42},{"key":"pmd","val":10}]},
  "fn": {"voice":"MONO","transpose":-12}
})";

int main() {
  std::printf("WebPatch Tests\n");

  Patch p;
  op4::webpatch::applyWebVar(p, juce::JSON::parse(kWebJson));

  // top level
  CHECK(p.algorithm == 5, "algorithm maps through");
  CHECK(p.feedback == 4, "feedback maps through");

  // operator 0: direct fields
  CHECK(p.operators[0].tl == 99, "lvl \"99\" -> tl 99");
  CHECK(p.operators[0].wave == 1, "wave \"W2 SQR\" -> 1");
  CHECK(p.operators[0].detune == 5, "det \"+2\" -> detune 5 (center 3)");
  CHECK(p.operators[0].coarse == 14, "ratio \"14.00\" -> coarse 14");
  CHECK(p.operators[0].fixed == 0, "fix false -> 0");

  // operator 1: unicode minus, fix true, sub-1 ratio
  CHECK(p.operators[1].detune == 0, "det \"\\u22124\" (Unicode minus) -> clamp 0");
  CHECK(p.operators[1].fixed == 1, "fix true -> 1");
  CHECK(p.operators[1].wave == 2, "wave \"W3 TRI\" -> 2");
  CHECK(p.operators[1].coarse == 0, "ratio \"0.50\" -> coarse 0 (0.5x slot)");

  // envelope wiring: a=0 -> fastest attack (ar 31); a=1 -> ar 0
  CHECK(p.operators[0].ar == 31, "ep.a 0.0 -> ar 31 (fast attack)");
  CHECK(p.operators[1].ar == 0,  "ep.a 1.0 -> ar 0 (slow attack)");
  CHECK(p.operators[0].d1l == 0, "ep.s 1.0 -> d1l 0 (full sustain)");
  CHECK(p.operators[0].rr == 15, "ep.r 0.0 -> rr 15 (fast release)");
  CHECK(p.operators[2].d1r == 16, "ep.d1 0.5 -> d1r 16");

  // lfo + function
  CHECK(p.lfo_wave == 2, "lfo wave \"TRI\" -> 2");
  CHECK(p.lfo_sync == 1, "lfo sync true -> 1");
  CHECK(p.lfo_speed == 42, "lfo knob speed 42 -> lfo_speed");
  CHECK(p.pmd == 10, "lfo knob pmd 10 -> pmd");
  CHECK(p.pms >= 4, "lfo PMD>0 nudges PMS so the LFO is audible");
  CHECK(p.poly_mono == 1, "fn voice MONO -> poly_mono 1");
  CHECK(p.transpose == 12, "fn transpose -12 -> 12 (center 24)");

  // native -> web -> native round-trip for the reachable fields.
  Patch n;
  n.algorithm = 6; n.feedback = 3;
  for (int i = 0; i < 4; ++i) {
    Operator& o = n.operators[i];
    o.tl = 90 - i * 10; o.detune = 3 + (i - 1);  // 2..5 in range
    o.wave = static_cast<uint8_t>(i); o.fixed = (i == 1) ? 1 : 0;
    o.coarse = static_cast<uint8_t>(i == 0 ? 0 : i);  // op0 -> 0.5x slot
    o.ar = 31 - i; o.d1r = 20 + i; o.d1l = i; o.rr = 8 + i;
  }
  n.lfo_wave = 2; n.lfo_sync = 1; n.lfo_speed = 42; n.lfo_delay = 5; n.pmd = 10; n.amd = 7;
  n.poly_mono = 1; n.transpose = 12;

  Patch back;  // start from defaults, overlay the round-tripped web var
  op4::webpatch::applyWebVar(back, op4::webpatch::toWebVar(n));

  bool ok = back.algorithm == n.algorithm && back.feedback == n.feedback &&
            back.lfo_wave == n.lfo_wave && back.lfo_sync == n.lfo_sync &&
            back.lfo_speed == n.lfo_speed && back.pmd == n.pmd &&
            back.poly_mono == n.poly_mono && back.transpose == n.transpose;
  for (int i = 0; i < 4; ++i) {
    const Operator& a = n.operators[i]; const Operator& b = back.operators[i];
    ok = ok && b.tl == a.tl && b.detune == a.detune && b.wave == a.wave &&
         b.fixed == a.fixed && b.coarse == a.coarse &&
         b.ar == a.ar && b.d1r == a.d1r && b.d1l == a.d1l && b.rr == a.rr;
  }
  CHECK(ok, "native -> web -> native preserves the reachable fields");

  std::printf(g_fail ? "\n%d CHECK(s) FAILED\n" : "\nALL CHECKS PASSED\n", g_fail);
  return g_fail ? 1 : 0;
}
