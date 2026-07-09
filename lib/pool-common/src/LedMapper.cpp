#include "LedMapper.h"

namespace pool {

LedMapper::LedMapper() : config_() {
  for (uint8_t i = 0; i < kLedCount; ++i) {
    config_.physicalOrder[i] = i;
  }
  config_.reverseLeft = false;
  config_.reverseRight = false;
}

LedMapper::LedMapper(const LedMapperConfig &config) : config_(config) {}

uint8_t LedMapper::physicalIndex(ChannelId channel, uint8_t segment) const {
  if (segment >= kLedsPerChannel) {
    return 0;
  }
  const bool reverse = channel == ChannelId::Left ? config_.reverseLeft
                                                   : config_.reverseRight;
  const uint8_t logicalSegment = reverse ? kLedsPerChannel - 1 - segment
                                         : segment;
  const uint8_t offset = channel == ChannelId::Left ? 0 : kLedsPerChannel;
  return config_.physicalOrder[offset + logicalSegment];
}

}  // namespace pool
