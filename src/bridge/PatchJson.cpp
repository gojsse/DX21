#include "PatchJson.h"

namespace op4::patchjson {

using juce::var;
using juce::DynamicObject;
using juce::Identifier;

// --- small helpers -------------------------------------------------------
static void put(DynamicObject* o, const char* k, int v) { o->setProperty(Identifier(k), v); }

static bool has(const var& v, const char* k) { return v.isObject() && v.hasProperty(Identifier(k)); }
static uint8_t u8(const var& v, const char* k, uint8_t def) {
  return has(v, k) ? static_cast<uint8_t>(static_cast<int>(v[Identifier(k)])) : def;
}

static var opToVar(const Operator& o) {
  auto* d = new DynamicObject();
  put(d, "ar", o.ar);   put(d, "d1r", o.d1r); put(d, "d2r", o.d2r); put(d, "rr", o.rr);
  put(d, "d1l", o.d1l); put(d, "tl", o.tl);   put(d, "rs", o.rs);   put(d, "ks", o.ks);
  put(d, "eg_shift", o.eg_shift); put(d, "am", o.am); put(d, "kvs", o.kvs); put(d, "ebs", o.ebs);
  put(d, "coarse", o.coarse); put(d, "fine", o.fine); put(d, "detune", o.detune);
  put(d, "fixed", o.fixed); put(d, "fixed_freq_range", o.fixed_freq_range); put(d, "wave", o.wave);
  put(d, "egl", o.egl); put(d, "eg2", o.eg2); put(d, "eg3", o.eg3);
  return var(d);
}

static void opOverlay(Operator& o, const var& v) {
  o.ar = u8(v, "ar", o.ar);   o.d1r = u8(v, "d1r", o.d1r); o.d2r = u8(v, "d2r", o.d2r); o.rr = u8(v, "rr", o.rr);
  o.d1l = u8(v, "d1l", o.d1l); o.tl = u8(v, "tl", o.tl);   o.rs = u8(v, "rs", o.rs);   o.ks = u8(v, "ks", o.ks);
  o.eg_shift = u8(v, "eg_shift", o.eg_shift); o.am = u8(v, "am", o.am); o.kvs = u8(v, "kvs", o.kvs);
  o.ebs = u8(v, "ebs", o.ebs);
  o.coarse = u8(v, "coarse", o.coarse); o.fine = u8(v, "fine", o.fine); o.detune = u8(v, "detune", o.detune);
  o.fixed = u8(v, "fixed", o.fixed); o.fixed_freq_range = u8(v, "fixed_freq_range", o.fixed_freq_range);
  o.wave = u8(v, "wave", o.wave);
  o.egl = u8(v, "egl", o.egl); o.eg2 = u8(v, "eg2", o.eg2); o.eg3 = u8(v, "eg3", o.eg3);
}

// --- public --------------------------------------------------------------
var toVar(const Patch& p) {
  auto* d = new DynamicObject();
  d->setProperty("name", juce::String(p.name));
  put(d, "algorithm", p.algorithm);
  put(d, "feedback", p.feedback);

  juce::Array<var> ops;
  for (const auto& o : p.operators) ops.add(opToVar(o));
  d->setProperty("operators", ops);

  auto* lfo = new DynamicObject();
  put(lfo, "speed", p.lfo_speed); put(lfo, "delay", p.lfo_delay);
  put(lfo, "pmd", p.pmd); put(lfo, "amd", p.amd); put(lfo, "sync", p.lfo_sync);
  put(lfo, "wave", p.lfo_wave); put(lfo, "pms", p.pms); put(lfo, "ams", p.ams);
  d->setProperty("lfo", var(lfo));

  auto* peg = new DynamicObject();
  put(peg, "r1", p.peg_r1); put(peg, "r2", p.peg_r2); put(peg, "r3", p.peg_r3);
  put(peg, "l0", p.peg_l0); put(peg, "l1", p.peg_l1); put(peg, "l2", p.peg_l2); put(peg, "l3", p.peg_l3);
  d->setProperty("pitchEg", var(peg));

  auto* fn = new DynamicObject();
  put(fn, "transpose", p.transpose); put(fn, "poly_mono", p.poly_mono);
  put(fn, "pb_range", p.pb_range); put(fn, "portamento_mode", p.portamento_mode);
  put(fn, "portamento_time", p.portamento_time); put(fn, "portamento_on", p.portamento_on);
  put(fn, "foot_mod_pitch", p.foot_mod_pitch); put(fn, "foot_mod_amp", p.foot_mod_amp);
  put(fn, "sustain", p.sustain); put(fn, "mw_pitch", p.mw_pitch); put(fn, "mw_amp", p.mw_amp);
  put(fn, "breath_pitch", p.breath_pitch); put(fn, "breath_amp", p.breath_amp);
  put(fn, "breath_bias", p.breath_bias); put(fn, "breath_eg", p.breath_eg);
  put(fn, "chorus", p.chorus); put(fn, "reverb", p.reverb);
  put(fn, "foot_ctrl_pitch", p.foot_ctrl_pitch); put(fn, "foot_ctrl_amp", p.foot_ctrl_amp);
  d->setProperty("fn", var(fn));

  return var(d);
}

void overlay(Patch& p, const var& v) {
  if (!v.isObject()) return;
  if (v.hasProperty("name")) p.name = v["name"].toString().toStdString();
  p.algorithm = u8(v, "algorithm", p.algorithm);
  p.feedback = u8(v, "feedback", p.feedback);

  if (v.hasProperty("operators")) {
    if (auto* arr = v["operators"].getArray())
      for (int i = 0; i < juce::jmin(4, arr->size()); ++i)
        opOverlay(p.operators[i], (*arr)[i]);
  }

  const var lfo = v.getProperty("lfo", var());
  p.lfo_speed = u8(lfo, "speed", p.lfo_speed); p.lfo_delay = u8(lfo, "delay", p.lfo_delay);
  p.pmd = u8(lfo, "pmd", p.pmd); p.amd = u8(lfo, "amd", p.amd); p.lfo_sync = u8(lfo, "sync", p.lfo_sync);
  p.lfo_wave = u8(lfo, "wave", p.lfo_wave); p.pms = u8(lfo, "pms", p.pms); p.ams = u8(lfo, "ams", p.ams);

  const var peg = v.getProperty("pitchEg", var());
  p.peg_r1 = u8(peg, "r1", p.peg_r1); p.peg_r2 = u8(peg, "r2", p.peg_r2); p.peg_r3 = u8(peg, "r3", p.peg_r3);
  p.peg_l0 = u8(peg, "l0", p.peg_l0); p.peg_l1 = u8(peg, "l1", p.peg_l1);
  p.peg_l2 = u8(peg, "l2", p.peg_l2); p.peg_l3 = u8(peg, "l3", p.peg_l3);

  const var fn = v.getProperty("fn", var());
  p.transpose = u8(fn, "transpose", p.transpose); p.poly_mono = u8(fn, "poly_mono", p.poly_mono);
  p.pb_range = u8(fn, "pb_range", p.pb_range); p.portamento_mode = u8(fn, "portamento_mode", p.portamento_mode);
  p.portamento_time = u8(fn, "portamento_time", p.portamento_time); p.portamento_on = u8(fn, "portamento_on", p.portamento_on);
  p.foot_mod_pitch = u8(fn, "foot_mod_pitch", p.foot_mod_pitch); p.foot_mod_amp = u8(fn, "foot_mod_amp", p.foot_mod_amp);
  p.sustain = u8(fn, "sustain", p.sustain); p.mw_pitch = u8(fn, "mw_pitch", p.mw_pitch); p.mw_amp = u8(fn, "mw_amp", p.mw_amp);
  p.breath_pitch = u8(fn, "breath_pitch", p.breath_pitch); p.breath_amp = u8(fn, "breath_amp", p.breath_amp);
  p.breath_bias = u8(fn, "breath_bias", p.breath_bias); p.breath_eg = u8(fn, "breath_eg", p.breath_eg);
  p.chorus = u8(fn, "chorus", p.chorus); p.reverb = u8(fn, "reverb", p.reverb);
  p.foot_ctrl_pitch = u8(fn, "foot_ctrl_pitch", p.foot_ctrl_pitch); p.foot_ctrl_amp = u8(fn, "foot_ctrl_amp", p.foot_ctrl_amp);
}

Patch fromVar(const var& v) { Patch p; overlay(p, v); return p; }

juce::String encode(const Patch& p) { return juce::JSON::toString(toVar(p)); }

Patch decode(const juce::String& json) { return fromVar(juce::JSON::parse(json)); }

}  // namespace op4::patchjson
