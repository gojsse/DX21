#include "VCEDCodec.h"
#include <algorithm>

namespace {
constexpr int kNameOff = 77;   // voice name: 10 ASCII bytes
constexpr int kNameLen = 10;

uint8_t clampByte(int v) { return static_cast<uint8_t>(std::clamp(v, 0, 127)); }
}  // namespace

void VCEDCodec::encodeOperator(std::vector<uint8_t>& d, int o, const Operator& op) {
  d[o + 0]  = op.ar  & 0x1f;   // Attack Rate 0-31
  d[o + 1]  = op.d1r & 0x1f;   // Decay 1 Rate
  d[o + 2]  = op.d2r & 0x1f;   // Decay 2 Rate
  d[o + 3]  = op.rr  & 0x0f;   // Release Rate 1-15
  d[o + 4]  = op.d1l & 0x0f;   // Decay 1 Level
  d[o + 5]  = clampByte(op.ks);   // Level Scaling 0-99
  d[o + 6]  = op.rs  & 0x03;   // Rate Scaling 0-3
  d[o + 7]  = op.ebs & 0x07;   // EG Bias Sensitivity 0-7
  d[o + 8]  = op.am  & 0x01;   // AM Enable
  d[o + 9]  = op.kvs & 0x07;   // Key Velocity Sensitivity
  d[o + 10] = clampByte(op.tl);   // Output Level 0-99
  d[o + 11] = op.coarse & 0x3f;   // Frequency (coarse ratio) 0-63
  d[o + 12] = op.detune & 0x07;   // Detune 0-6 (3 = center)
}

Operator VCEDCodec::decodeOperator(const std::vector<uint8_t>& d, int o) {
  Operator op;
  op.ar = d[o + 0]; op.d1r = d[o + 1]; op.d2r = d[o + 2]; op.rr = d[o + 3];
  op.d1l = d[o + 4]; op.ks = d[o + 5]; op.rs = d[o + 6]; op.ebs = d[o + 7];
  op.am = d[o + 8]; op.kvs = d[o + 9]; op.tl = d[o + 10];
  op.coarse = d[o + 11]; op.detune = d[o + 12];
  return op;
}

std::vector<uint8_t> VCEDCodec::encode(const Patch& p, uint8_t deviceNumber) {
  std::vector<uint8_t> data(DataLength, 0);

  // operators, order OP4, OP2, OP3, OP1
  encodeOperator(data, 0,  p.operators[3]);
  encodeOperator(data, 13, p.operators[1]);
  encodeOperator(data, 26, p.operators[2]);
  encodeOperator(data, 39, p.operators[0]);

  // common
  data[52] = p.algorithm & 0x07;
  data[53] = p.feedback & 0x07;
  data[54] = clampByte(p.lfo_speed);
  data[55] = clampByte(p.lfo_delay);
  data[56] = clampByte(p.pmd);
  data[57] = clampByte(p.amd);
  data[58] = p.lfo_sync & 0x01;
  data[59] = p.lfo_wave & 0x03;
  data[60] = p.pms & 0x07;
  data[61] = p.ams & 0x03;
  data[62] = clampByte(p.transpose);     // 0-48
  data[63] = p.poly_mono & 0x01;
  data[64] = clampByte(p.pb_range);      // 0-12
  data[65] = p.portamento_mode & 0x01;
  data[66] = clampByte(p.portamento_time);
  data[67] = clampByte(p.foot_mod_amp);  // Foot Control Volume
  data[68] = p.sustain & 0x01;
  data[69] = p.portamento_on & 0x01;
  data[70] = p.chorus & 0x01;
  data[71] = clampByte(p.mw_pitch);
  data[72] = clampByte(p.mw_amp);
  data[73] = clampByte(p.breath_pitch);
  data[74] = clampByte(p.breath_amp);
  data[75] = clampByte(p.breath_bias);
  data[76] = clampByte(p.breath_eg);

  // name: 10 ASCII, space-padded
  for (int i = 0; i < kNameLen; ++i)
    data[kNameOff + i] = static_cast<uint8_t>(i < static_cast<int>(p.name.size()) ? p.name[i] : ' ');

  data[87] = clampByte(p.peg_r1); data[88] = clampByte(p.peg_r2); data[89] = clampByte(p.peg_r3);
  data[90] = clampByte(p.peg_l1); data[91] = clampByte(p.peg_l2); data[92] = clampByte(p.peg_l3);

  std::vector<uint8_t> out;
  out.reserve(TotalLength);
  out.insert(out.end(), {0xF0, 0x43, static_cast<uint8_t>(deviceNumber & 0x0f), 0x03, 0x00, 0x5D});
  out.insert(out.end(), data.begin(), data.end());
  out.push_back(computeChecksum(data.data(), DataLength));
  out.push_back(0xF7);
  return out;
}

Patch VCEDCodec::decode(const std::vector<uint8_t>& sysex) {
  Patch p;
  if (!isVCED(sysex)) return p;
  const std::vector<uint8_t> d(sysex.begin() + HeaderLength, sysex.begin() + HeaderLength + DataLength);

  p.operators[3] = decodeOperator(d, 0);
  p.operators[1] = decodeOperator(d, 13);
  p.operators[2] = decodeOperator(d, 26);
  p.operators[0] = decodeOperator(d, 39);

  p.algorithm = d[52]; p.feedback = d[53];
  p.lfo_speed = d[54]; p.lfo_delay = d[55]; p.pmd = d[56]; p.amd = d[57];
  p.lfo_sync = d[58]; p.lfo_wave = d[59]; p.pms = d[60]; p.ams = d[61];
  p.transpose = d[62]; p.poly_mono = d[63]; p.pb_range = d[64];
  p.portamento_mode = d[65]; p.portamento_time = d[66]; p.foot_mod_amp = d[67];
  p.sustain = d[68]; p.portamento_on = d[69]; p.chorus = d[70];
  p.mw_pitch = d[71]; p.mw_amp = d[72];
  p.breath_pitch = d[73]; p.breath_amp = d[74]; p.breath_bias = d[75]; p.breath_eg = d[76];

  std::string name(d.begin() + kNameOff, d.begin() + kNameOff + kNameLen);
  while (!name.empty() && name.back() == ' ') name.pop_back();  // trim trailing pad
  p.name = name;

  p.peg_r1 = d[87]; p.peg_r2 = d[88]; p.peg_r3 = d[89];
  p.peg_l1 = d[90]; p.peg_l2 = d[91]; p.peg_l3 = d[92];
  return p;
}

bool VCEDCodec::isVCED(const std::vector<uint8_t>& s) {
  return s.size() == static_cast<size_t>(TotalLength) &&
         s[0] == 0xF0 && s[1] == 0x43 && (s[2] & 0xf0) == 0x00 &&
         s[3] == 0x03 && s[4] == 0x00 && s[5] == 0x5D &&
         s.back() == 0xF7;
}

bool VCEDCodec::validateChecksum(const std::vector<uint8_t>& s) {
  if (!isVCED(s)) return false;
  return computeChecksum(&s[HeaderLength], DataLength) == s[HeaderLength + DataLength];
}

uint8_t VCEDCodec::computeChecksum(const uint8_t* data, int len) {
  uint8_t sum = 0;
  for (int i = 0; i < len; ++i) sum += data[i];
  return (128 - (sum & 0x7F)) & 0x7F;  // two's complement, 7-bit
}
