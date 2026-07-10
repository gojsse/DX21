#pragma once
#include <vector>
#include <cstdint>
#include "../model/Patch.h"

// VCED — Single Voice (DX21/DX27/DX100/TX81Z base)
// Format: F0 43 0n 03 00 5D <93 bytes> <checksum> F7

class VCEDCodec {
public:
  static constexpr int DataLength = 93;
  static constexpr int TotalLength = 7 + DataLength + 1;  // header + data + checksum + F7

  // Serialize Patch → VCED sysex bytes
  static std::vector<uint8_t> encode(const Patch& patch, uint8_t deviceNumber = 0);

  // Deserialize VCED sysex → Patch
  // Returns empty Patch on error
  static Patch decode(const std::vector<uint8_t>& sysex);

  // Validate checksum (returns true if valid, offers "import anyway" option in UI)
  static bool validateChecksum(const std::vector<uint8_t>& sysex);

  // Compute checksum over data bytes
  static uint8_t computeChecksum(const std::vector<uint8_t>& dataBytes);

private:
  // Operator block layout (OP4, OP2, OP3, OP1 order, 13 bytes each)
  static void encodeOperator(std::vector<uint8_t>& data, int offset, const Operator& op);
  static Operator decodeOperator(const std::vector<uint8_t>& data, int offset);
};
