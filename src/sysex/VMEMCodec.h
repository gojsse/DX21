#pragma once
#include <array>
#include <vector>
#include <cstdint>
#include "../model/Patch.h"

// VMEM — 32-Voice Packed Bank (DX21/DX27/DX100/TX81Z).
//   F0 43 0n 04 20 00 <4096 data bytes> <checksum> F7   (0x2000 = 4096, 7-bit)
// 32 voices x 128 bytes, bit-packed.
//
// !!! STRUCTURAL / [verify] !!!  The exact hardware bit layout must be derived
// from a factory bank dump (handoff §2.3: unpack, re-pack, assert byte-identical
// against a fixture). The packVoice/unpackVoice map below is a *documented,
// internally-lossless* placeholder: it round-trips Patch[32] <-> VMEM perfectly
// (so the librarian/bank plumbing is real and testable), but the byte/bit
// positions are NOT yet confirmed against hardware. It is centralized in one
// place so correcting it is a table edit, not a rewrite.
class VMEMCodec {
public:
  static constexpr int Voices = 32;
  static constexpr int VoiceBytes = 128;
  static constexpr int HeaderLength = 6;                    // F0 43 0n 04 20 00
  static constexpr int DataLength = Voices * VoiceBytes;    // 4096
  static constexpr int TotalLength = HeaderLength + DataLength + 2;  // + checksum + F7

  using PatchBank = std::array<Patch, Voices>;

  static std::vector<uint8_t> encode(const PatchBank& bank, uint8_t deviceNumber = 0);
  static PatchBank decode(const std::vector<uint8_t>& sysex);

  static bool isVMEM(const std::vector<uint8_t>& sysex);
  static bool validateChecksum(const std::vector<uint8_t>& sysex);
  static uint8_t computeChecksum(const uint8_t* data, int len);

private:
  static void packVoice(uint8_t* v, const Patch& p);     // 128 bytes  [verify]
  static Patch unpackVoice(const uint8_t* v);
};
