#pragma once
#include <vector>
#include <cstdint>
#include "../model/Patch.h"

// VCED — Single Voice (DX21/DX27/DX100 native; TX81Z base layer).
//   F0 43 0n 03 00 5D <93 data bytes> <checksum> F7
// Operators are transmitted OP4, OP2, OP3, OP1 (13 bytes each); see handoff
// §2.1 for the byte table. Byte-level details are [verify] against a fixture.
class VCEDCodec {
public:
  static constexpr int HeaderLength = 6;   // F0 43 0n 03 00 5D
  static constexpr int DataLength = 93;
  static constexpr int TotalLength = HeaderLength + DataLength + 2;  // + checksum + F7

  static std::vector<uint8_t> encode(const Patch& patch, uint8_t deviceNumber = 0);

  // Returns a default Patch if the buffer isn't a well-formed VCED message.
  static Patch decode(const std::vector<uint8_t>& sysex);

  // True if the buffer looks like a VCED message (header + length), ignoring
  // the checksum (real-world .syx files often have bad checksums).
  static bool isVCED(const std::vector<uint8_t>& sysex);

  static bool validateChecksum(const std::vector<uint8_t>& sysex);
  static uint8_t computeChecksum(const uint8_t* data, int len);

private:
  static void encodeOperator(std::vector<uint8_t>& data, int off, const Operator& op);
  static Operator decodeOperator(const std::vector<uint8_t>& data, int off);
};
