#pragma once
#include <cstdint>

// CCTable — reserved default MIDI CC -> parameter map (M4). The UI's right-click
// "assign CC" flow (a known-open item) will edit a live copy of this.
namespace op4 {

struct CCAssignment {
  uint8_t cc = 0;         // MIDI controller number
  int16_t paramIndex = -1;  // -1 = unassigned
};

// Standard controllers we always honour regardless of assignment.
constexpr uint8_t kCC_ModWheel   = 1;
constexpr uint8_t kCC_BreathCtrl = 2;
constexpr uint8_t kCC_FootCtrl   = 4;
constexpr uint8_t kCC_Sustain    = 64;

// TODO(M4): default assignment table + user overrides.

}  // namespace op4
