#pragma once
#include <vector>
#include <cstdint>
#include "../model/Patch.h"

// VMEM — 32-Voice Packed Bank (TX81Z native)
// Format: F0 43 0n 04 20 00 <4096 bytes> <checksum> F7

class VMEMCodec {
public:
  static constexpr int BankSize = 32;
  static constexpr int VoiceDataLength = 128;  // per voice, bit-packed
  static constexpr int TotalDataLength = BankSize * VoiceDataLength;

  using PatchBank = std::array<Patch, BankSize>;

  // Encode PatchBank → VMEM sysex (4096 bytes)
  static std::vector<uint8_t> encode(const PatchBank& bank, uint8_t deviceNumber = 0);

  // Decode VMEM sysex → PatchBank
  static PatchBank decode(const std::vector<uint8_t>& sysex);

  // Validate checksum
  static bool validateChecksum(const std::vector<uint8_t>& sysex);

  static uint8_t computeChecksum(const std::vector<uint8_t>& dataBytes);

private:
  // Bit-field packing/unpacking (VMEM is heavily bit-packed)
  // Reference: empirical reverse-engineering from hardware dumps
  static void encodeVoice(std::vector<uint8_t>& data, int voiceIndex, const Patch& patch);
  static Patch decodeVoice(const std::vector<uint8_t>& data, int voiceIndex);
};
