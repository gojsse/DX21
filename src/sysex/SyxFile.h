#pragma once
#include <cstdint>
#include <vector>

// Split a .syx file blob into individual complete sysex messages (F0 ... F7).
// Tolerant of junk between messages and an unterminated trailing F0. JUCE-free.
namespace op4 {

inline std::vector<std::vector<uint8_t>> splitSysex(const uint8_t* d, int n) {
  std::vector<std::vector<uint8_t>> out;
  int i = 0;
  while (i < n) {
    if (d[i] != 0xF0) { ++i; continue; }
    int j = i + 1;
    while (j < n && d[j] != 0xF7) ++j;
    if (j >= n) break;                     // unterminated final message
    out.emplace_back(d + i, d + j + 1);    // inclusive of F0..F7
    i = j + 1;
  }
  return out;
}

}  // namespace op4
