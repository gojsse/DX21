#pragma once
// Minimal PCM16 WAV read/write — JUCE-free, just enough for the null-test
// harness (golden regression + hardware A/B). Interleaved stereo assumed.
#include <cstdint>
#include <cstring>
#include <fstream>
#include <string>
#include <vector>

namespace op4::wav {

inline void putU32(std::vector<uint8_t>& b, uint32_t v) {
  b.push_back(v & 0xff); b.push_back((v >> 8) & 0xff);
  b.push_back((v >> 16) & 0xff); b.push_back((v >> 24) & 0xff);
}
inline void putU16(std::vector<uint8_t>& b, uint16_t v) {
  b.push_back(v & 0xff); b.push_back((v >> 8) & 0xff);
}
inline void putTag(std::vector<uint8_t>& b, const char* t) {
  for (int i = 0; i < 4; ++i) b.push_back(static_cast<uint8_t>(t[i]));
}

// Write interleaved int16 samples (numFrames * channels values) as PCM16 WAV.
inline bool writePCM16(const std::string& path, const std::vector<int16_t>& interleaved,
                       int sampleRate, int channels) {
  const uint32_t dataBytes = static_cast<uint32_t>(interleaved.size() * sizeof(int16_t));
  std::vector<uint8_t> h;
  putTag(h, "RIFF"); putU32(h, 36 + dataBytes); putTag(h, "WAVE");
  putTag(h, "fmt "); putU32(h, 16); putU16(h, 1); putU16(h, static_cast<uint16_t>(channels));
  putU32(h, static_cast<uint32_t>(sampleRate));
  putU32(h, static_cast<uint32_t>(sampleRate * channels * 2));
  putU16(h, static_cast<uint16_t>(channels * 2)); putU16(h, 16);
  putTag(h, "data"); putU32(h, dataBytes);

  std::ofstream f(path, std::ios::binary);
  if (!f) return false;
  f.write(reinterpret_cast<const char*>(h.data()), static_cast<std::streamsize>(h.size()));
  f.write(reinterpret_cast<const char*>(interleaved.data()), dataBytes);
  return static_cast<bool>(f);
}

// Read a PCM16 WAV's "data" chunk into interleaved int16. Returns false on I/O
// error or unsupported format. Scans chunks so extra metadata is tolerated.
inline bool readPCM16(const std::string& path, std::vector<int16_t>& out,
                      int& sampleRate, int& channels) {
  std::ifstream f(path, std::ios::binary);
  if (!f) return false;
  std::vector<uint8_t> b((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
  if (b.size() < 44 || std::memcmp(b.data(), "RIFF", 4) != 0 || std::memcmp(b.data() + 8, "WAVE", 4) != 0)
    return false;

  auto u32 = [&](size_t o) { return uint32_t(b[o]) | (uint32_t(b[o+1]) << 8) | (uint32_t(b[o+2]) << 16) | (uint32_t(b[o+3]) << 24); };
  auto u16 = [&](size_t o) { return uint16_t(b[o]) | (uint16_t(b[o+1]) << 8); };

  size_t pos = 12;
  sampleRate = 0; channels = 0;
  while (pos + 8 <= b.size()) {
    const char* id = reinterpret_cast<const char*>(b.data() + pos);
    const uint32_t sz = u32(pos + 4);
    const size_t body = pos + 8;
    if (std::memcmp(id, "fmt ", 4) == 0 && body + 16 <= b.size()) {
      channels = u16(body + 2);
      sampleRate = static_cast<int>(u32(body + 4));
    } else if (std::memcmp(id, "data", 4) == 0) {
      const size_t n = std::min<size_t>(sz, b.size() - body) / 2;
      out.resize(n);
      for (size_t i = 0; i < n; ++i) out[i] = static_cast<int16_t>(u16(body + i * 2));
      return channels > 0 && sampleRate > 0;
    }
    pos = body + sz + (sz & 1);  // chunks are word-aligned
  }
  return false;
}

}  // namespace op4::wav
