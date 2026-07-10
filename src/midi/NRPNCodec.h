#pragma once
#include <cstdint>
#include "../model/Patch.h"

// NRPN address space for OP4 (M4)
// Full parameter addressing via NRPN MSB/LSB + data entry

class NRPNCodec {
public:
  // Map NRPN address (MSB, LSB) → parameter index
  static bool isValidAddress(uint8_t msb, uint8_t lsb);

  // Encode parameter change as NRPN CC sequence
  // Returns MIDI CCs: CC99 (MSB), CC98 (LSB), CC6 (data)
  static void encodeNRPN(uint8_t paramIndex, uint8_t value,
                         uint8_t& cc99, uint8_t& cc98, uint8_t& cc6);

  // Decode NRPN CCs → parameter index + value
  static bool decodeNRPN(uint8_t cc99, uint8_t cc98, uint8_t cc6,
                         uint8_t& paramIndex, uint8_t& value);

  // Parameter count (all addressable params via NRPN)
  static constexpr int ParamCount = 256;  // TODO: define actual count
};
