#pragma once
#include <vector>
#include <cstdint>
#include "../model/Patch.h"

// ACED — TX81Z Additional Voice Data (extends VCED)
// Format: F0 43 0n 7E 00 21 "LM  8976AE" <23 bytes> <checksum> F7

class ACEDCodec {
public:
  static constexpr int DataLength = 33;  // includes "LM  8976AE" (10 bytes) + data (23 bytes)

  // Encode Patch TX81Z extensions → ACED sysex
  static std::vector<uint8_t> encode(const Patch& patch, uint8_t deviceNumber = 0);

  // Decode ACED → Patch (updates operator fixed-freq, waveforms, and TX81Z params)
  static Patch decode(const std::vector<uint8_t>& sysex, Patch& targetPatch);

  // Validate checksum
  static bool validateChecksum(const std::vector<uint8_t>& sysex);

  static uint8_t computeChecksum(const std::vector<uint8_t>& dataBytes);
};
