#include "ButtonDebouncer.h"

namespace pool {

ButtonDebouncer::ButtonDebouncer(uint32_t debounceMs, bool initialHigh)
    : debounceMs_(debounceMs),
      stableHigh_(initialHigh),
      lastRawHigh_(initialHigh),
      lastRawChangeMs_(0) {}

bool ButtonDebouncer::poll(bool rawHigh, uint32_t now) {
  if (rawHigh != lastRawHigh_) {
    lastRawHigh_ = rawHigh;
    lastRawChangeMs_ = now;
  }

  if (stableHigh_ != lastRawHigh_ &&
      static_cast<uint32_t>(now - lastRawChangeMs_) >= debounceMs_) {
    stableHigh_ = lastRawHigh_;
    return !stableHigh_;
  }
  return false;
}

bool ButtonDebouncer::isPressed() const { return !stableHigh_; }

}  // namespace pool
