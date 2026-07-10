#include "VCEDCodec.h"
#include <cstring>
#include <algorithm>

std::vector<uint8_t> VCEDCodec::encode(const Patch& patch, uint8_t deviceNumber) {
  std::vector<uint8_t> sysex;
  sysex.reserve(TotalLength);

  // Header: F0 43 0n 03 00 5D
  sysex.push_back(0xF0);
  sysex.push_back(0x43);
  sysex.push_back(deviceNumber & 0x0F);
  sysex.push_back(0x03);
  sysex.push_back(0x00);
  sysex.push_back(0x5D);  // length high byte
  sysex.push_back(0x00);  // length low byte (93 = 0x5D)

  // TODO: Encode 93 data bytes in order:
  // - Operators (OP4, OP2, OP3, OP1 order, 13 bytes each = 52 bytes)
  // - Common parameters (52–92)
  // See technical handoff §2.1 for byte mapping

  std::vector<uint8_t> data(DataLength, 0);
  // encodeOperator(data, 0, patch.operators[3]);   // OP4 at offset 0
  // encodeOperator(data, 13, patch.operators[1]);  // OP2 at offset 13
  // encodeOperator(data, 26, patch.operators[2]);  // OP3 at offset 26
  // encodeOperator(data, 39, patch.operators[0]);  // OP1 at offset 39
  // data[52] = patch.algorithm; ...

  sysex.insert(sysex.end(), data.begin(), data.end());

  // Checksum
  uint8_t checksum = computeChecksum(data);
  sysex.push_back(checksum);

  // End: F7
  sysex.push_back(0xF7);

  return sysex;
}

Patch VCEDCodec::decode(const std::vector<uint8_t>& sysex) {
  Patch patch;
  if (sysex.size() != TotalLength) return patch;

  // TODO: validate header, extract 93 data bytes, parse operators + common
  // See technical handoff §2.1 for byte mapping

  return patch;
}

bool VCEDCodec::validateChecksum(const std::vector<uint8_t>& sysex) {
  if (sysex.size() != TotalLength) return false;
  if (sysex[0] != 0xF0 || sysex[sysex.size() - 1] != 0xF7) return false;

  std::vector<uint8_t> data(sysex.begin() + 7, sysex.begin() + 7 + DataLength);
  uint8_t expected = sysex[7 + DataLength];
  return computeChecksum(data) == expected;
}

uint8_t VCEDCodec::computeChecksum(const std::vector<uint8_t>& dataBytes) {
  uint8_t sum = 0;
  for (auto byte : dataBytes) sum += byte;
  return (128 - (sum & 0x7F)) & 0x7F;
}

void VCEDCodec::encodeOperator(std::vector<uint8_t>& data, int offset, const Operator& op) {
  // TODO: pack operator into 13 bytes at offset
  data[offset + 0] = op.ar;
  data[offset + 1] = op.d1r;
  // ... etc
}

Operator VCEDCodec::decodeOperator(const std::vector<uint8_t>& data, int offset) {
  Operator op;
  // TODO: unpack operator from 13 bytes at offset
  op.ar = data[offset + 0];
  op.d1r = data[offset + 1];
  // ... etc
  return op;
}
