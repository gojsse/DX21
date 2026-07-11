#include "ACEDCodec.h"
#include <algorithm>
#include <array>

namespace {
// classification string "LM  8976AE"
constexpr std::array<uint8_t, 10> kClass = {'L', 'M', ' ', ' ', '8', '9', '7', '6', 'A', 'E'};
uint8_t clampByte(int v) { return static_cast<uint8_t>(std::clamp(v, 0, 127)); }
// per-op data offsets within the 23-byte data block, in OP4/OP2/OP3/OP1 order
constexpr int kOpOff[4] = {0, 5, 10, 15};  // indexes patch.operators[3],[1],[2],[0]
constexpr int kOpIndex[4] = {3, 1, 2, 0};
}  // namespace

std::vector<uint8_t> ACEDCodec::encode(const Patch& p, uint8_t deviceNumber) {
  std::vector<uint8_t> body(BodyLength, 0);
  std::copy(kClass.begin(), kClass.end(), body.begin());
  uint8_t* d = body.data() + ClassLength;  // 23-byte data block

  for (int k = 0; k < 4; ++k) {
    const Operator& op = p.operators[kOpIndex[k]];
    const int o = kOpOff[k];
    d[o + 0] = op.fixed & 0x01;
    d[o + 1] = op.fixed_freq_range & 0x07;
    d[o + 2] = op.fine & 0x0f;          // Frequency Range Fine 0-15
    d[o + 3] = op.wave & 0x07;          // Operator Waveform W1-W8
    d[o + 4] = op.eg_shift & 0x03;
  }
  d[20] = p.reverb & 0x07;
  d[21] = clampByte(p.foot_ctrl_pitch);
  d[22] = clampByte(p.foot_ctrl_amp);

  std::vector<uint8_t> out;
  out.reserve(TotalLength);
  out.insert(out.end(), {0xF0, 0x43, static_cast<uint8_t>(deviceNumber & 0x0f), 0x7E, 0x00, 0x21});
  out.insert(out.end(), body.begin(), body.end());
  out.push_back(computeChecksum(body.data(), BodyLength));
  out.push_back(0xF7);
  return out;
}

bool ACEDCodec::apply(const std::vector<uint8_t>& s, Patch& p) {
  if (!isACED(s)) return false;
  const uint8_t* d = &s[HeaderLength + ClassLength];  // 23-byte data block

  for (int k = 0; k < 4; ++k) {
    Operator& op = p.operators[kOpIndex[k]];
    const int o = kOpOff[k];
    op.fixed = d[o + 0]; op.fixed_freq_range = d[o + 1]; op.fine = d[o + 2];
    op.wave = d[o + 3]; op.eg_shift = d[o + 4];
  }
  p.reverb = d[20]; p.foot_ctrl_pitch = d[21]; p.foot_ctrl_amp = d[22];
  return true;
}

bool ACEDCodec::isACED(const std::vector<uint8_t>& s) {
  if (s.size() != static_cast<size_t>(TotalLength)) return false;
  if (s[0] != 0xF0 || s[1] != 0x43 || (s[2] & 0xf0) != 0x00 ||
      s[3] != 0x7E || s[4] != 0x00 || s[5] != 0x21 || s.back() != 0xF7)
    return false;
  for (int i = 0; i < ClassLength; ++i)
    if (s[HeaderLength + i] != kClass[static_cast<size_t>(i)]) return false;
  return true;
}

bool ACEDCodec::validateChecksum(const std::vector<uint8_t>& s) {
  if (!isACED(s)) return false;
  return computeChecksum(&s[HeaderLength], BodyLength) == s[HeaderLength + BodyLength];
}

uint8_t ACEDCodec::computeChecksum(const uint8_t* body, int len) {
  uint8_t sum = 0;
  for (int i = 0; i < len; ++i) sum += body[i];
  return (128 - (sum & 0x7F)) & 0x7F;
}
