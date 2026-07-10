#pragma once
#include <cstdint>

// Shared enums / constants for the OP4 model layer.
namespace op4 {

// Which hardware family the engine emulates. Selects the OPZ "DX21 mask"
// (sine-only, no fixed-frequency/ACED) vs full TX81Z behaviour.
enum class Machine : uint8_t { DX21, DX27, DX100, TX81Z };

inline bool isDX21Family(Machine m) { return m != Machine::TX81Z; }

// Per-operator LFO waveform (OPZ). 0=saw 1=square 2=triangle 3=sample&hold.
enum class LfoWave : uint8_t { Saw, Square, Triangle, SampleHold };

constexpr int kOperatorsPerVoice = 4;
constexpr int kAlgorithms = 8;

}  // namespace op4
