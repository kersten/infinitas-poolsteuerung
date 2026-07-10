#pragma once

#include <stdint.h>

namespace poolanode {

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

}  // namespace poolanode
