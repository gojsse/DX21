#include "VMEMCodec.h"
#include <algorithm>

// ============================================================================
// Packed 128-byte voice layout  [verify against a factory bank fixture]
//
// Operators are stored OP4, OP2, OP3, OP1 (10 bytes each = 40 bytes):
//   +0 AR(5)  +1 D1R(5)  +2 D2R(5)  +3 RR(4)  +4 D1L(4)  +5 LS(7)
//   +6 [7:AME][6-4:EBS][1-0:RS]      +7 [6-4:KVS][2-0:detune]
//   +8 TL(7)  +9 coarse(6)
// Common (bytes 40-65):
//   40 [2-0:alg][5-3:feedback]      41 lfo_speed  42 lfo_delay
//   43 pmd  44 amd  45 [7:sync][6-5:wave][4-2:pms][1-0:ams]
//   46 transpose  47 [0:poly][4-1:pb_range][5:porta_mode]  48 porta_time
//   49 foot_vol  50 [0:sustain][1:porta_on][2:chorus]
//   51 mw_pitch  52 mw_amp  53 breath_pitch  54 breath_amp
//   55 breath_bias  56 breath_eg
//   57-59 peg_r1..3   60-62 peg_l1..3   63 reverb  64 foot_ctrl_pitch  65 foot_ctrl_amp
// ACED per-op (OP4/2/3/1), 2 bytes each = 8 bytes, at 66-73:
//   +0 [0:fixed][3-1:fix_range][5-4:eg_shift]   +1 [2-0:wave][6-3:fine]
// Name: 74-83 (10 ASCII).  84-127 reserved (0).
// ============================================================================

namespace {
constexpr int kOpBytes = 10;
constexpr int kOpIndex[4] = {3, 1, 2, 0};   // storage slot k -> patch operator (OP4/2/3/1)
constexpr int kAcedOff = 66;
constexpr int kNameOff = 74;
uint8_t clampByte(int v) { return static_cast<uint8_t>(std::clamp(v, 0, 127)); }
}  // namespace

void VMEMCodec::packVoice(uint8_t* v, const Patch& p) {
  std::fill(v, v + VoiceBytes, uint8_t{0});

  for (int k = 0; k < 4; ++k) {
    const Operator& o = p.operators[kOpIndex[k]];
    uint8_t* b = v + k * kOpBytes;
    b[0] = o.ar & 0x1f; b[1] = o.d1r & 0x1f; b[2] = o.d2r & 0x1f;
    b[3] = o.rr & 0x0f; b[4] = o.d1l & 0x0f; b[5] = clampByte(o.ks);
    b[6] = static_cast<uint8_t>(((o.am & 1) << 7) | ((o.ebs & 7) << 4) | (o.rs & 3));
    b[7] = static_cast<uint8_t>(((o.kvs & 7) << 4) | (o.detune & 7));
    b[8] = clampByte(o.tl); b[9] = o.coarse & 0x3f;
  }

  v[40] = static_cast<uint8_t>((p.algorithm & 7) | ((p.feedback & 7) << 3));
  v[41] = clampByte(p.lfo_speed); v[42] = clampByte(p.lfo_delay);
  v[43] = clampByte(p.pmd); v[44] = clampByte(p.amd);
  v[45] = static_cast<uint8_t>(((p.lfo_sync & 1) << 7) | ((p.lfo_wave & 3) << 5) | ((p.pms & 7) << 2) | (p.ams & 3));
  v[46] = clampByte(p.transpose);
  v[47] = static_cast<uint8_t>((p.poly_mono & 1) | ((p.pb_range & 0x0f) << 1) | ((p.portamento_mode & 1) << 5));
  v[48] = clampByte(p.portamento_time);
  v[49] = clampByte(p.foot_mod_amp);
  v[50] = static_cast<uint8_t>((p.sustain & 1) | ((p.portamento_on & 1) << 1) | ((p.chorus & 1) << 2));
  v[51] = clampByte(p.mw_pitch); v[52] = clampByte(p.mw_amp);
  v[53] = clampByte(p.breath_pitch); v[54] = clampByte(p.breath_amp);
  v[55] = clampByte(p.breath_bias); v[56] = clampByte(p.breath_eg);
  v[57] = clampByte(p.peg_r1); v[58] = clampByte(p.peg_r2); v[59] = clampByte(p.peg_r3);
  v[60] = clampByte(p.peg_l1); v[61] = clampByte(p.peg_l2); v[62] = clampByte(p.peg_l3);
  v[63] = p.reverb & 7; v[64] = clampByte(p.foot_ctrl_pitch); v[65] = clampByte(p.foot_ctrl_amp);

  for (int k = 0; k < 4; ++k) {
    const Operator& o = p.operators[kOpIndex[k]];
    uint8_t* b = v + kAcedOff + k * 2;
    b[0] = static_cast<uint8_t>((o.fixed & 1) | ((o.fixed_freq_range & 7) << 1) | ((o.eg_shift & 3) << 4));
    b[1] = static_cast<uint8_t>((o.wave & 7) | ((o.fine & 0x0f) << 3));
  }

  for (int i = 0; i < 10; ++i)
    v[kNameOff + i] = static_cast<uint8_t>(i < static_cast<int>(p.name.size()) ? p.name[i] : ' ');
}

Patch VMEMCodec::unpackVoice(const uint8_t* v) {
  Patch p;
  for (int k = 0; k < 4; ++k) {
    Operator& o = p.operators[kOpIndex[k]];
    const uint8_t* b = v + k * kOpBytes;
    o.ar = b[0]; o.d1r = b[1]; o.d2r = b[2]; o.rr = b[3]; o.d1l = b[4]; o.ks = b[5];
    o.am = (b[6] >> 7) & 1; o.ebs = (b[6] >> 4) & 7; o.rs = b[6] & 3;
    o.kvs = (b[7] >> 4) & 7; o.detune = b[7] & 7;
    o.tl = b[8]; o.coarse = b[9] & 0x3f;
  }

  p.algorithm = v[40] & 7; p.feedback = (v[40] >> 3) & 7;
  p.lfo_speed = v[41]; p.lfo_delay = v[42]; p.pmd = v[43]; p.amd = v[44];
  p.lfo_sync = (v[45] >> 7) & 1; p.lfo_wave = (v[45] >> 5) & 3; p.pms = (v[45] >> 2) & 7; p.ams = v[45] & 3;
  p.transpose = v[46];
  p.poly_mono = v[47] & 1; p.pb_range = (v[47] >> 1) & 0x0f; p.portamento_mode = (v[47] >> 5) & 1;
  p.portamento_time = v[48]; p.foot_mod_amp = v[49];
  p.sustain = v[50] & 1; p.portamento_on = (v[50] >> 1) & 1; p.chorus = (v[50] >> 2) & 1;
  p.mw_pitch = v[51]; p.mw_amp = v[52]; p.breath_pitch = v[53]; p.breath_amp = v[54];
  p.breath_bias = v[55]; p.breath_eg = v[56];
  p.peg_r1 = v[57]; p.peg_r2 = v[58]; p.peg_r3 = v[59];
  p.peg_l1 = v[60]; p.peg_l2 = v[61]; p.peg_l3 = v[62];
  p.reverb = v[63] & 7; p.foot_ctrl_pitch = v[64]; p.foot_ctrl_amp = v[65];

  for (int k = 0; k < 4; ++k) {
    Operator& o = p.operators[kOpIndex[k]];
    const uint8_t* b = v + kAcedOff + k * 2;
    o.fixed = b[0] & 1; o.fixed_freq_range = (b[0] >> 1) & 7; o.eg_shift = (b[0] >> 4) & 3;
    o.wave = b[1] & 7; o.fine = (b[1] >> 3) & 0x0f;
  }

  std::string name(reinterpret_cast<const char*>(v + kNameOff), 10);
  while (!name.empty() && name.back() == ' ') name.pop_back();
  p.name = name;
  return p;
}

std::vector<uint8_t> VMEMCodec::encode(const PatchBank& bank, uint8_t deviceNumber) {
  std::vector<uint8_t> data(DataLength, 0);
  for (int i = 0; i < Voices; ++i)
    packVoice(data.data() + i * VoiceBytes, bank[static_cast<size_t>(i)]);

  std::vector<uint8_t> out;
  out.reserve(TotalLength);
  out.insert(out.end(), {0xF0, 0x43, static_cast<uint8_t>(deviceNumber & 0x0f), 0x04, 0x20, 0x00});
  out.insert(out.end(), data.begin(), data.end());
  out.push_back(computeChecksum(data.data(), DataLength));
  out.push_back(0xF7);
  return out;
}

VMEMCodec::PatchBank VMEMCodec::decode(const std::vector<uint8_t>& sysex) {
  PatchBank bank;
  if (!isVMEM(sysex)) return bank;
  const uint8_t* data = &sysex[HeaderLength];
  for (int i = 0; i < Voices; ++i)
    bank[static_cast<size_t>(i)] = unpackVoice(data + i * VoiceBytes);
  return bank;
}

bool VMEMCodec::isVMEM(const std::vector<uint8_t>& s) {
  return s.size() == static_cast<size_t>(TotalLength) &&
         s[0] == 0xF0 && s[1] == 0x43 && (s[2] & 0xf0) == 0x00 &&
         s[3] == 0x04 && s[4] == 0x20 && s[5] == 0x00 && s.back() == 0xF7;
}

bool VMEMCodec::validateChecksum(const std::vector<uint8_t>& s) {
  if (!isVMEM(s)) return false;
  return computeChecksum(&s[HeaderLength], DataLength) == s[HeaderLength + DataLength];
}

uint8_t VMEMCodec::computeChecksum(const uint8_t* data, int len) {
  uint8_t sum = 0;
  for (int i = 0; i < len; ++i) sum += data[i];
  return (128 - (sum & 0x7F)) & 0x7F;
}
