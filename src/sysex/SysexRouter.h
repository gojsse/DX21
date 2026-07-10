#pragma once
#include <cstdint>
#include <vector>

// SysexRouter — dispatches an incoming sysex block to the right codec by
// inspecting the Yamaha header. Pure data in / out; no JUCE coupling (M2).
//
//   VCED  F0 43 0n 03 00 5D ...   single voice
//   ACED  F0 43 0n 7E 00 21 "LM  8976AE" ...  TX81Z additional voice data
//   VMEM  F0 43 0n 04 20 00 ...   32-voice bank
namespace op4 {

enum class SysexKind { Unknown, VCED, ACED, VMEM };

inline SysexKind classifySysex(const std::vector<uint8_t>& m) {
  if (m.size() < 6 || m.front() != 0xF0 || m[1] != 0x43) return SysexKind::Unknown;
  const uint8_t fmt = m[3];
  if (fmt == 0x03) return SysexKind::VCED;
  if (fmt == 0x04) return SysexKind::VMEM;
  if (fmt == 0x7E) return SysexKind::ACED;  // narrow further on "LM  8976AE" in M2
  return SysexKind::Unknown;
}

// TODO(M2): route() that decodes into a Patch / PatchBank via the codecs.

}  // namespace op4
