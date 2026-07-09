#pragma once

#include <stdint.h>

namespace pool {

enum class ChannelId : uint8_t {
  Left,
  Right,
};

enum class ChannelState : uint8_t {
  Idle,
  Starting,
  Running,
  CanceledFeedback,
  FinishedFeedback,
};

const char *channelStateName(ChannelState state);
bool parseChannelState(const char *text, ChannelState &state);

bool isChannelActive(ChannelState state);

}  // namespace pool
