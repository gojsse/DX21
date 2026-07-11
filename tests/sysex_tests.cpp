// VCED codec round-trip tests (M2). No fixtures required: proves the codec is
// internally consistent (Patch -> bytes -> Patch, and byte-exact re-encode) and
// that the checksum validates. Byte-accuracy vs real hardware is gated by
// private fixtures (tests/fixtures/*.syx) — [verify].

#include "sysex/VCEDCodec.h"
#include "sysex/ACEDCodec.h"
#include "sysex/SysexRouter.h"
#include "sysex/SysexFifo.h"
#include "sysex/SyxFile.h"
#include "sysex/VMEMCodec.h"
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

  // --- SysexRouter: combines ACED + VCED into one voice ---
  {
    op4::SysexRouter router;
    CHECK(!router.feed(aced).has_value(), "router: ACED alone yields no patch (stashed)");
    auto out = router.feed(bytes);  // VCED for `src` (no TX fields)
    CHECK(out.has_value(), "router: VCED after ACED yields a patch");
    // the stashed ACED (from `tx`) should be overlaid onto the VCED voice
    CHECK(out && out->reverb == tx.reverb && out->operators[0].wave == tx.operators[0].wave,
          "router: ACED fields merged into the following VCED");
  }
  {
    op4::SysexRouter router;
    auto out = router.feed(bytes);  // standalone VCED
    CHECK(out.has_value() && out->name == src.name, "router: standalone VCED yields a patch");
    CHECK(op4::classifySysex(aced) == op4::SysexKind::ACED &&
          op4::classifySysex(bytes) == op4::SysexKind::VCED, "classifySysex tags VCED/ACED");
  }

  // --- SysexFifo: lock-free hand-off (audio -> message thread) ---
  {
    op4::SysexFifo fifo;
    CHECK(fifo.push(aced.data(), (int)aced.size()) && fifo.push(bytes.data(), (int)bytes.size()),
          "fifo: push ACED then VCED");
    std::vector<uint8_t> m1, m2, m3;
    CHECK(fifo.pop(m1) && m1 == aced, "fifo: FIFO order preserved (ACED first)");
    CHECK(fifo.pop(m2) && m2 == bytes, "fifo: second message is the VCED");
    CHECK(!fifo.pop(m3), "fifo: empty after draining");
    std::vector<uint8_t> tooBig(op4::SysexFifo::kSlotBytes + 1, 0);
    CHECK(!fifo.push(tooBig.data(), (int)tooBig.size()), "fifo: oversized message rejected");
  }

  // --- .syx file splitting (ACED + VCED concatenated, with junk padding) ---
  {
    std::vector<uint8_t> file;
    file.push_back(0x00);  // leading junk
    file.insert(file.end(), aced.begin(), aced.end());
    file.insert(file.end(), bytes.begin(), bytes.end());
    file.push_back(0xF0);  // unterminated trailing F0 (ignored)
    auto msgs = op4::splitSysex(file.data(), (int)file.size());
    CHECK(msgs.size() == 2 && msgs[0] == aced && msgs[1] == bytes,
          "splitSysex extracts the two messages, ignoring junk");

    op4::SysexRouter r;
    std::optional<Patch> got;
    for (auto& m : msgs) if (auto p = r.feed(m)) got = p;
    CHECK(got && got->reverb == tx.reverb, "loaded voice from the split file is the full TX81Z voice");
  }

  // --- VMEM (32-voice bank) — structural round-trip ([verify] byte layout) ---
  {
    VMEMCodec::PatchBank bank;
    for (int vi = 0; vi < VMEMCodec::Voices; ++vi) {
      Patch& b = bank[vi];
      b = vcedPatch();  // VCED-representable fields
      b.name = "VOICE" + std::to_string(vi);
      b.algorithm = vi % 8; b.feedback = vi % 8;
      for (int i = 0; i < 4; ++i) {  // add the TX81Z/ACED fields VMEM also stores
        Operator& o = b.operators[i];
        o.fixed = (vi + i) & 1; o.fixed_freq_range = (vi + i) % 8;
        o.fine = (vi * 2 + i) % 16; o.wave = (vi + i) % 8; o.eg_shift = (vi + i) % 4;
      }
      b.reverb = vi % 8; b.foot_ctrl_pitch = vi % 100; b.foot_ctrl_amp = (vi * 3) % 100;
    }

    const std::vector<uint8_t> vmem = VMEMCodec::encode(bank, 0);
    CHECK(static_cast<int>(vmem.size()) == VMEMCodec::TotalLength, "VMEM length is 4104");
    CHECK(vmem[3] == 0x04 && vmem[4] == 0x20 && vmem[5] == 0x00, "VMEM header (04 20 00 = 4096)");
    CHECK(VMEMCodec::isVMEM(vmem) && VMEMCodec::validateChecksum(vmem), "VMEM header + checksum");

    const VMEMCodec::PatchBank back = VMEMCodec::decode(vmem);
    bool bankOk = true;
    for (int vi = 0; vi < VMEMCodec::Voices; ++vi) bankOk = bankOk && (back[vi] == bank[vi]);
    CHECK(bankOk, "PatchBank[32] -> VMEM -> PatchBank[32] is identity");
    CHECK(VMEMCodec::encode(back, 0) == vmem, "VMEM -> bank -> VMEM is byte-identical");
  }

  std::printf(g_fail ? "\n%d CHECK(s) FAILED\n" : "\nALL CHECKS PASSED\n", g_fail);
  return g_fail ? 1 : 0;
}
