#pragma once
#include <algorithm>

// Pure controller-curve helpers (JUCE-free, so they're unit-tested directly).
// The breath controller (CC2) rests at 0, so — like the DX/TX81Z — it only
// affects a note when the patch enables it (breath_amp / breath_pitch > 0);
// otherwise these return the neutral value and the note plays normally.
namespace op4::ctl {

// Breath -> amplitude gain (0..1) applied to carriers. breathAmp == 0 => 1.0
// (patch ignores breath). Otherwise breath rides from a floor (breath_bias) at
// CC2=0 up to full at CC2=127, blended in by the breath_amp sensitivity.
// Ranges assumed 0..99 (amp) / 0..100 (bias); [verify] against hardware.
inline float breathAmpGain(int breathAmp, int breathBias, int ccBreath) {
  if (breathAmp <= 0) return 1.0f;
  const float sens    = std::clamp(breathAmp / 99.0f, 0.0f, 1.0f);
  const float bias01  = std::clamp(breathBias / 100.0f, 0.0f, 1.0f);
  const float breath  = std::clamp(ccBreath / 127.0f, 0.0f, 1.0f);
  const float level   = bias01 + breath * (1.0f - bias01);  // floor at bias, -> 1
  return 1.0f - sens * (1.0f - level);                      // sens 0 -> 1, sens 1 -> level
}

// Breath -> pitch in semitones. breathPitch == 0 => 0. Up to ~an octave at full
// sensitivity + full breath. [verify].
inline float breathPitchSemis(int breathPitch, int ccBreath) {
  if (breathPitch <= 0) return 0.0f;
  const float amt      = std::clamp(ccBreath / 127.0f, 0.0f, 1.0f);
  const float maxSemis = std::clamp(breathPitch / 99.0f, 0.0f, 1.0f) * 12.0f;
  return amt * maxSemis;
}

}  // namespace op4::ctl
