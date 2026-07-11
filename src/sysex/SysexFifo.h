#pragma once
#include <array>
#include <atomic>
#include <cstdint>
#include <cstring>
#include <vector>

// SysexFifo — a lock-free single-producer/single-consumer ring for handing
// incoming sysex from the audio thread to the message thread, so processBlock
// only *copies* bytes (no decoding / allocation). Messages larger than a slot
// (e.g. VMEM banks) are dropped here and handled via a non-realtime path.
namespace op4 {

class SysexFifo {
public:
  static constexpr int kSlots = 16;
  static constexpr int kSlotBytes = 320;  // fits VCED (101) + ACED (41)

  // audio thread: copy a message in. Returns false if full or too large.
  bool push(const uint8_t* data, int len) {
    if (len <= 0 || len > kSlotBytes) return false;
    const int w = write_.load(std::memory_order_relaxed);
    const int next = (w + 1) % kSlots;
    if (next == read_.load(std::memory_order_acquire)) return false;  // full
    std::memcpy(slots_[w].data(), data, static_cast<size_t>(len));
    len_[w] = len;
    write_.store(next, std::memory_order_release);
    return true;
  }

  // message thread: pop the next message, false if empty.
  bool pop(std::vector<uint8_t>& out) {
    const int r = read_.load(std::memory_order_relaxed);
    if (r == write_.load(std::memory_order_acquire)) return false;  // empty
    out.assign(slots_[r].begin(), slots_[r].begin() + len_[r]);
    read_.store((r + 1) % kSlots, std::memory_order_release);
    return true;
  }

private:
  std::array<std::array<uint8_t, kSlotBytes>, kSlots> slots_{};
  std::array<int, kSlots> len_{};
  std::atomic<int> write_{0}, read_{0};
};

}  // namespace op4
