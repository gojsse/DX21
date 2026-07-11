// VCED codec round-trip tests (M2). No fixtures required: proves the codec is
// internally consistent (Patch -> bytes -> Patch, and byte-exact re-encode) and
// that the checksum validates. Byte-accuracy vs real hardware is gated by
// private fixtures (tests/fixtures/*.syx) — [verify].

#include "sysex/VCEDCodec.h"
#include "sysex/ACEDCodec.h"
#include "model/Patch.h"
#include <cstdio>
#include <vector>

static int g_fail = 0;
#define CHECK(c, m) do { if (!(c)) { std::printf("FAIL: %s\n", m); ++g_fail; } \
                         else std::printf("ok:   %s\n", m); } while (0)

// A patch that sets only VCED-representable fields (non-VCED fields stay at
// their defaults, which is what decode() produces), so the Patch round-trip is
// an exact identity.
static Patch vcedPatch() {
  Patch p;
  p.name = "BRASS 1";
  p.algorithm = 5; p.feedback = 6;
  for (int i = 0; i < 4; ++i) {
    Operator& o = p.operators[i];
    o.ar = 31 - i; o.d1r = 20 + i; o.d2r = 5 + i; o.rr = 8 + i; o.d1l = 2 + i;
    o.ks = 40 + i; o.rs = i % 4; o.ebs = i % 8; o.am = i & 1; o.kvs = (i + 2) % 8;
    o.tl = 99 - i * 7; o.coarse = i * 3 + 1; o.detune = (i + 1) % 7;
  }
  p.lfo_speed = 42; p.lfo_delay = 10; p.pmd = 30; p.amd = 12;
  p.lfo_sync = 1; p.lfo_wave = 2; p.pms = 5; p.ams = 2;
  p.transpose = 24; p.poly_mono = 1; p.pb_range = 12;
  p.portamento_mode = 1; p.portamento_time = 33; p.foot_mod_amp = 44;
  p.sustain = 1; p.portamento_on = 1; p.chorus = 1;
  p.mw_pitch = 50; p.mw_amp = 60; p.breath_pitch = 70; p.breath_amp = 80;
  p.breath_bias = 55; p.breath_eg = 25;
  p.peg_r1 = 90; p.peg_r2 = 80; p.peg_r3 = 70;
  p.peg_l1 = 60; p.peg_l2 = 50; p.peg_l3 = 40;
  return p;
}

int main() {
  std::printf("OP4 Sysex Tests (VCED, M2)\n");

  const Patch src = vcedPatch();
  const std::vector<uint8_t> bytes = VCEDCodec::encode(src, /*device=*/0);

  CHECK(static_cast<int>(bytes.size()) == VCEDCodec::TotalLength, "encoded length is 101");
  CHECK(bytes[0] == 0xF0 && bytes[1] == 0x43 && bytes[3] == 0x03 &&
        bytes[4] == 0x00 && bytes[5] == 0x5D && bytes.back() == 0xF7, "VCED header + F7");
  CHECK(VCEDCodec::isVCED(bytes), "isVCED recognises the message");
  CHECK(VCEDCodec::validateChecksum(bytes), "checksum validates");

  const Patch back = VCEDCodec::decode(bytes);
  CHECK(back == src, "Patch -> VCED -> Patch is identity");

  const std::vector<uint8_t> bytes2 = VCEDCodec::encode(back, 0);
  CHECK(bytes2 == bytes, "VCED -> Patch -> VCED is byte-identical");

  // corrupting a data byte should break the checksum
  std::vector<uint8_t> bad = bytes;
  bad[10] ^= 0x01;
  CHECK(!VCEDCodec::validateChecksum(bad), "checksum catches a flipped bit");

  // --- ACED (TX81Z extensions) ---
  Patch tx = vcedPatch();
  for (int i = 0; i < 4; ++i) {
    Operator& o = tx.operators[i];
    o.fixed = i & 1; o.fixed_freq_range = i % 8; o.fine = (i * 3) % 16;
    o.wave = (i + 2) % 8; o.eg_shift = i % 4;
  }
  tx.reverb = 5; tx.foot_ctrl_pitch = 33; tx.foot_ctrl_amp = 77;

  const std::vector<uint8_t> aced = ACEDCodec::encode(tx, 0);
  CHECK(static_cast<int>(aced.size()) == ACEDCodec::TotalLength, "ACED length is 41");
  CHECK(ACEDCodec::isACED(aced) && ACEDCodec::validateChecksum(aced), "ACED header/class + checksum");

  // apply ACED onto a VCED-decoded patch and confirm the TX81Z fields land
  Patch merged = VCEDCodec::decode(VCEDCodec::encode(tx, 0));  // VCED half (no TX fields)
  CHECK(merged.reverb == 0 && merged.operators[0].wave == 0, "VCED alone omits TX81Z fields");
  CHECK(ACEDCodec::apply(aced, merged), "ACED applies");
  bool txOk = merged.reverb == tx.reverb && merged.foot_ctrl_pitch == tx.foot_ctrl_pitch &&
              merged.foot_ctrl_amp == tx.foot_ctrl_amp;
  for (int i = 0; i < 4; ++i) {
    const Operator& a = tx.operators[i]; const Operator& b = merged.operators[i];
    txOk = txOk && b.fixed == a.fixed && b.fixed_freq_range == a.fixed_freq_range &&
           b.fine == a.fine && b.wave == a.wave && b.eg_shift == a.eg_shift;
  }
  CHECK(txOk, "VCED + ACED reconstructs the full TX81Z voice");

  // ACED byte round-trip
  Patch fresh; ACEDCodec::apply(aced, fresh);
  CHECK(ACEDCodec::encode(fresh, 0) == aced, "ACED -> Patch -> ACED is byte-identical");

  std::printf(g_fail ? "\n%d CHECK(s) FAILED\n" : "\nALL CHECKS PASSED\n", g_fail);
  return g_fail ? 1 : 0;
}
