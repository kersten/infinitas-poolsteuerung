#pragma once

#include <stdint.h>

namespace pool {

// Debounces an active-low, INPUT_PULLUP button. poll() returns true once per
// physical press and never repeats while the button remains held.
class ButtonDebouncer {
 public:
  explicit ButtonDebouncer(uint32_t debounceMs = 35, bool initialHigh = true);

  bool poll(bool rawHigh, uint32_t now);
  bool isPressed() const;

 private:
  uint32_t debounceMs_;
  bool stableHigh_;
  bool lastRawHigh_;
  uint32_t lastRawChangeMs_;
};

}  // namespace pool
