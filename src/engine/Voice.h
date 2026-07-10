#pragma once
#include <cstdint>

// Voice — a channel-allocation record, NOT a DSP owner.
//
// ymfm emulates the whole 8-channel OPZ in one object (see OPZChip), so a
// "voice" here is just the bookkeeping that maps a held MIDI note to one chip
// channel: which channel, what note, whether it's sounding or in release, and
// how old it is (for oldest-note stealing).
struct Voice {
  int  channel = -1;        // owning OPZChip channel (0-7), -1 = unassigned
  int  note = -1;           // MIDI note currently held
  float velocity = 0.0f;
  bool active = false;      // key held (excludes release tail)
  bool releasing = false;   // note-off sent, envelope tail still sounding
  uint64_t startTick = 0;   // allocation order, for voice stealing

  void reset() {
    note = -1;
    velocity = 0.0f;
    active = false;
    releasing = false;
    startTick = 0;
  }
};
