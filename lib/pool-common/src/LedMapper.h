#pragma once

#include <stdint.h>

#include "ChannelState.h"

namespace pool {

static const uint8_t kLedCount = 16;
static const uint8_t kLedsPerChannel = 8;

struct LedMapperConfig {
  uint8_t physicalOrder[kLedCount];
  bool reverseLeft;
  bool reverseRight;
};

class LedMapper {
 public:
  LedMapper();
  explicit LedMapper(const LedMapperConfig &config);

  uint8_t physicalIndex(ChannelId channel, uint8_t segment) const;

 private:
  LedMapperConfig config_;
};

}  // namespace pool
