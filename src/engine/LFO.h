#pragma once

// LFO — placeholder. The OPZ has two internal LFOs driven directly by patch
// registers (see OPZChip: reg 0x18/0x19 rate/AM depth, 0x38 PMS/AMS), so voice
// pitch/amp modulation is handled in-chip for authenticity.
//
// This host-side LFO is reserved for modulation the chip can't source itself
// (e.g. a global mod-matrix destination in Modern mode). Not wired in M1.
class LFO {
  // TODO(M4+): host-side modulation source for the mod matrix.
};
