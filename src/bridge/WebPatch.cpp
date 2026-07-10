#include "WebPatch.h"
#include <cmath>

namespace op4::webpatch {

using juce::var;
using juce::String;
using juce::Identifier;

static bool has(const var& v, const char* k) { return v.isObject() && v.hasProperty(Identifier(k)); }
static String str(const var& v, const char* k) { return v[Identifier(k)].toString(); }

static uint8_t clampU8(int v, int lo, int hi) {
  return static_cast<uint8_t>(juce::jlimit(lo, hi, v));
}

// "+2" / "-4" / "−4" -> signed int. Robust to any minus-like glyph the UI may
// use (ASCII '-', U+2212 MINUS SIGN, en/em dash): any non-digit, non-'+' char
// is treated as a negative sign.
static int parseDetune(const String& s) {
  auto cp = s.getCharPointer();
  bool negative = false;
  String digits;
  for (juce::juce_wchar c; (c = cp.getAndAdvance()) != 0;) {
    if (c >= '0' && c <= '9') digits << c;
    else if (c != '+' && c != ' ') negative = true;
  }
  const int v = digits.getIntValue();
  return negative ? -v : v;
}

// "W2 SQR" -> 1 (W1..W8 -> 0..7)
static uint8_t parseWave(const String& s) {
  const String t = s.trim().toUpperCase();
  if (t.startsWithChar('W')) return clampU8(t.substring(1).getIntValue() - 1, 0, 7);
  return 0;
}

// "SAW"/"SQR"/"TRI"/"S&H" -> 0..3 (matches Patch.h lfo_wave)
static uint8_t parseLfoWave(const String& s) {
  const String t = s.trim().toUpperCase();
  if (t.startsWith("SQ")) return 1;
  if (t.startsWith("TR")) return 2;
  if (t.startsWith("S&") || t.startsWith("SH") || t.startsWith("S/H")) return 3;
  return 0;  // saw
}

// ADSR graph value (0..1) -> chip rate/level (higher rate = faster).
// [verify] linear approximation; real DX curves are non-linear.
static uint8_t rate01(double graph01, int maxVal) {
  return clampU8(static_cast<int>(std::lround((1.0 - graph01) * maxVal)), 0, maxVal);
}

static void applyWebOp(Operator& o, const var& wv) {
  if (!wv.isObject()) return;
  if (has(wv, "lvl"))  o.tl = clampU8(str(wv, "lvl").getIntValue(), 0, 99);      // DX output level
  if (has(wv, "wave")) o.wave = parseWave(str(wv, "wave"));
  if (has(wv, "fix"))  o.fixed = static_cast<bool>(wv["fix"]) ? 1 : 0;
  if (has(wv, "det"))  o.detune = clampU8(3 + parseDetune(str(wv, "det")), 0, 7);  // center = 3 [verify]
  if (has(wv, "ratio")) {
    // [verify] ratio-string -> coarse index. <1.0 collapses to the 0.5x slot
    // (OPM MULTIPLE 0); the real DX ratio table refines this + the fine nibble.
    const double r = str(wv, "ratio").getFloatValue();
    o.coarse = (r < 1.0) ? 0 : clampU8(static_cast<int>(std::lround(r)), 1, 31);
  }
  const var ep = wv.getProperty("ep", var());
  if (ep.isObject()) {
    auto g = [&](const char* k) { return static_cast<double>(ep.getProperty(Identifier(k), 0.0)); };
    o.ar  = rate01(g("a"),  31);
    o.d1r = rate01(g("d1"), 31);
    o.d1l = rate01(g("s"),  15);  // sustain level: high sustain -> low D1L
    o.rr  = rate01(g("r"),  15);
  }
}

// --- inverse: native -> web display -------------------------------------
static String detuneStr(uint8_t detune) {
  const int v = static_cast<int>(detune) - 3;  // center 3
  return (v >= 0 ? "+" : "-") + String(std::abs(v));
}
static String ratioStr(uint8_t coarse) {
  return coarse == 0 ? String("0.50") : String(static_cast<int>(coarse)) + ".00";
}
static const char* lfoWaveStr(uint8_t w) {
  switch (w) { case 1: return "SQR"; case 2: return "TRI"; case 3: return "S&H"; default: return "SAW"; }
}
static double graph01(uint8_t v, int maxVal) { return 1.0 - static_cast<double>(v) / maxVal; }

var toWebVar(const Patch& p) {
  auto* d = new juce::DynamicObject();
  d->setProperty("algorithm", static_cast<int>(p.algorithm));
  d->setProperty("feedback", static_cast<int>(p.feedback));

  juce::Array<var> ops;
  for (const auto& o : p.operators) {
    auto* od = new juce::DynamicObject();
    od->setProperty("lvl", String(static_cast<int>(o.tl)));
    od->setProperty("det", detuneStr(o.detune));
    od->setProperty("wave", "W" + String(static_cast<int>(o.wave) + 1));  // name suffix is UI cosmetic
    od->setProperty("fix", static_cast<bool>(o.fixed));
    od->setProperty("ratio", ratioStr(o.coarse));
    auto* ep = new juce::DynamicObject();
    ep->setProperty("a",  graph01(o.ar,  31));
    ep->setProperty("d1", graph01(o.d1r, 31));
    ep->setProperty("s",  graph01(o.d1l, 15));
    ep->setProperty("r",  graph01(o.rr,  15));
    od->setProperty("ep", var(ep));
    ops.add(var(od));
  }
  d->setProperty("ops", ops);

  auto* lfo = new juce::DynamicObject();
  lfo->setProperty("sync", static_cast<bool>(p.lfo_sync));
  lfo->setProperty("wave", lfoWaveStr(p.lfo_wave));
  juce::Array<var> knobs;
  auto knob = [&](const char* key, int val) {
    auto* k = new juce::DynamicObject();
    k->setProperty("key", key); k->setProperty("val", val);
    knobs.add(var(k));
  };
  knob("speed", p.lfo_speed); knob("delay", p.lfo_delay); knob("pmd", p.pmd); knob("amd", p.amd);
  lfo->setProperty("knobs", knobs);
  d->setProperty("lfo", var(lfo));

  auto* fn = new juce::DynamicObject();
  fn->setProperty("voice", p.poly_mono ? "MONO" : "POLY");
  fn->setProperty("transpose", static_cast<int>(p.transpose) - 24);  // native centers at 24
  d->setProperty("fn", var(fn));

  return var(d);
}

void applyWebVar(Patch& p, const var& web) {
  if (!web.isObject()) return;

  if (has(web, "name")) p.name = web["name"].toString().toStdString();
  if (has(web, "algorithm")) p.algorithm = clampU8(static_cast<int>(web["algorithm"]), 0, 7);
  if (has(web, "feedback"))  p.feedback = clampU8(static_cast<int>(web["feedback"]), 0, 7);

  if (const auto* ops = web.getProperty("ops", var()).getArray())
    for (int i = 0; i < juce::jmin(4, ops->size()); ++i)
      applyWebOp(p.operators[i], (*ops)[i]);

  const var lfo = web.getProperty("lfo", var());
  if (lfo.isObject()) {
    if (has(lfo, "wave")) p.lfo_wave = parseLfoWave(str(lfo, "wave"));
    if (has(lfo, "sync")) p.lfo_sync = static_cast<bool>(lfo["sync"]) ? 1 : 0;
    if (const auto* knobs = lfo.getProperty("knobs", var()).getArray())
      for (const auto& k : *knobs) {
        const String key = str(k, "key");
        const int val = static_cast<int>(k.getProperty("val", 0));
        if (key == "speed")      p.lfo_speed = clampU8(val, 0, 99);
        else if (key == "delay") p.lfo_delay = clampU8(val, 0, 99);
        else if (key == "pmd")   p.pmd = clampU8(val, 0, 99);
        else if (key == "amd")   p.amd = clampU8(val, 0, 99);
      }
  }

  const var fn = web.getProperty("fn", var());
  if (fn.isObject()) {
    if (has(fn, "voice")) p.poly_mono = (str(fn, "voice").trim().toUpperCase() == "MONO") ? 1 : 0;
    // web transpose is signed semitones; native centers at 24 (0..48).
    if (has(fn, "transpose")) p.transpose = clampU8(24 + static_cast<int>(fn["transpose"]), 0, 48);
  }

  // The web LFO exposes depth (PMD/AMD) but not per-op sensitivity (PMS/AMS);
  // assume a musical sensitivity so the LFO is actually audible from the UI.
  // [verify]
  if (p.pmd > 0 && p.pms == 0) p.pms = 4;
  if (p.amd > 0 && p.ams == 0) p.ams = 2;
}

}  // namespace op4::webpatch
