#include "LedMapper.h"

namespace poolanode {

LedMapper::LedMapper() : config_() {
  for (uint8_t index = 0; index < kAnodeLedCount; ++index) {
    config_.physicalOrder[index] = index;
  }
  config_.reverseSwimmingPool = false;
  config_.reverseWhirlpool = false;
}

LedMapper::LedMapper(const LedMapperConfig &config) : config_(config) {}

uint8_t LedMapper::physicalIndex(AnodeChannelId channel,
                                 uint8_t segment) const {
  if (segment >= kLedsPerAnodeChannel) {
    return 0;
  }
  const bool reverse = channel == AnodeChannelId::SwimmingPool
                           ? config_.reverseSwimmingPool
                           : config_.reverseWhirlpool;
  const uint8_t logicalSegment =
      reverse ? kLedsPerAnodeChannel - 1 - segment : segment;
  const uint8_t offset = channel == AnodeChannelId::SwimmingPool
                             ? 0
                             : kLedsPerAnodeChannel;
  return config_.physicalOrder[offset + logicalSegment];
}

}  // namespace poolanode
