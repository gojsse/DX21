#pragma once
#include <vector>
#include <cstdint>
#include "../model/Patch.h"

// ACED — TX81Z Additional Voice Data. Carries the TX81Z-only fields that VCED
// omits (fixed frequency, per-op waveform, EG shift, reverb, foot control).
//   F0 43 0n 7E 00 21 "LM  8976AE" <23 data bytes> <checksum> F7
// The TX81Z transmits ACED immediately before VCED, so ACED *overlays* onto a
// patch (typically one just decoded from VCED). Field order is [verify].
class ACEDCodec {
public:
  static constexpr int HeaderLength = 6;    // F0 43 0n 7E 00 21
  static constexpr int ClassLength = 10;    // "LM  8976AE"
  static constexpr int DataLength = 23;     // 5*4 per-op + 3 common
  static constexpr int BodyLength = ClassLength + DataLength;  // 33 = 0x21 (checksummed)
  static constexpr int TotalLength = HeaderLength + BodyLength + 2;  // + checksum + F7

  static std::vector<uint8_t> encode(const Patch& patch, uint8_t deviceNumber = 0);

  // Overlay the ACED fields onto an existing patch. Returns false if the buffer
  // isn't a well-formed ACED message (patch left untouched).
  static bool apply(const std::vector<uint8_t>& sysex, Patch& target);

  static bool isACED(const std::vector<uint8_t>& sysex);
  static bool validateChecksum(const std::vector<uint8_t>& sysex);
  static uint8_t computeChecksum(const uint8_t* body, int len);
};
