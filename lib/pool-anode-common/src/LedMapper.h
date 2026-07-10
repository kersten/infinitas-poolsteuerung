#pragma once

#include <stdint.h>

#include "AnodeState.h"

namespace poolanode {

static const uint8_t kAnodeLedCount = 16;
static const uint8_t kLedsPerAnodeChannel = 8;

struct LedMapperConfig {
  uint8_t physicalOrder[kAnodeLedCount];
  bool reverseSwimmingPool;
  bool reverseWhirlpool;
};

class LedMapper {
 public:
  LedMapper();
  explicit LedMapper(const LedMapperConfig &config);

  uint8_t physicalIndex(AnodeChannelId channel, uint8_t segment) const;

 private:
  LedMapperConfig config_;
};

}  // namespace poolanode
