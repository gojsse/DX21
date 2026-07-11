#pragma once
#include <cstdint>
#include <optional>
#include <vector>
#include "../model/Patch.h"
#include "VCEDCodec.h"
#include "ACEDCodec.h"

// SysexRouter — feeds complete sysex messages in and yields a Patch when a full
// single voice has arrived. The TX81Z transmits ACED immediately followed by
// VCED, so an ACED is stashed and merged into the next VCED; a standalone VCED
// also yields a patch. Header-only + JUCE-free (unit-tested in op4_sysex_tests).
//
// VMEM (32-voice banks) has a different shape (a whole bank) and gets its own
// path in VMEMCodec — this router is the single-voice funnel.
namespace op4 {

enum class SysexKind { Unknown, VCED, ACED, VMEM };

inline SysexKind classifySysex(const std::vector<uint8_t>& m) {
  if (m.size() < 6 || m.front() != 0xF0 || m[1] != 0x43) return SysexKind::Unknown;
  switch (m[3]) {
    case 0x03: return SysexKind::VCED;
    case 0x04: return SysexKind::VMEM;
    case 0x7E: return SysexKind::ACED;  // classification checked by ACEDCodec::isACED
    default:   return SysexKind::Unknown;
  }
}

class SysexRouter {
public:
  // Feed one complete sysex message. Returns a fully-assembled Patch when ready
  // (a VCED, with any immediately-preceding ACED overlaid); ACED alone stashes
  // and returns nullopt.
  std::optional<Patch> feed(const std::vector<uint8_t>& msg) {
    if (ACEDCodec::isACED(msg)) {
      pendingAced_ = msg;
      hasPendingAced_ = true;
      return std::nullopt;
    }
    if (VCEDCodec::isVCED(msg)) {
      Patch p = VCEDCodec::decode(msg);
      if (hasPendingAced_) {
        ACEDCodec::apply(pendingAced_, p);
        hasPendingAced_ = false;
      }
      return p;
    }
    return std::nullopt;  // VMEM / unknown handled elsewhere
  }

  void reset() { hasPendingAced_ = false; }

private:
  std::vector<uint8_t> pendingAced_;
  bool hasPendingAced_ = false;
};

}  // namespace op4
