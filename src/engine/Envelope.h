#pragma once

// Envelope — placeholder. Operator envelopes (AR/D1R/D2R/RR/D1L) run inside the
// OPZ per-operator and are programmed via registers in OPZChip, so there is no
// host-side EG in Vintage mode.
//
// Reserved for a higher-resolution host EG in Modern mode (oversampled, full
// float precision) if we later offer a non-authentic path. Not wired in M1.
class Envelope {
  // TODO(M1 Modern): optional host-side full-resolution EG.
};
