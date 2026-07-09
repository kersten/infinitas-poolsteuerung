#include "ChannelState.h"

#include <string.h>

namespace pool {

const char *channelStateName(ChannelState state) {
  switch (state) {
    case ChannelState::Idle:
      return "IDLE";
    case ChannelState::Starting:
      return "STARTING";
    case ChannelState::Running:
      return "RUNNING";
    case ChannelState::CanceledFeedback:
      return "CANCELED_FEEDBACK";
    case ChannelState::FinishedFeedback:
      return "FINISHED_FEEDBACK";
  }
  return "IDLE";
}

bool parseChannelState(const char *text, ChannelState &state) {
  if (text == 0) {
    return false;
  }
  if (strcmp(text, "IDLE") == 0) {
    state = ChannelState::Idle;
  } else if (strcmp(text, "STARTING") == 0) {
    state = ChannelState::Starting;
  } else if (strcmp(text, "RUNNING") == 0) {
    state = ChannelState::Running;
  } else if (strcmp(text, "CANCELED_FEEDBACK") == 0) {
    state = ChannelState::CanceledFeedback;
  } else if (strcmp(text, "FINISHED_FEEDBACK") == 0) {
    state = ChannelState::FinishedFeedback;
  } else {
    return false;
  }
  return true;
}

bool isChannelActive(ChannelState state) {
  return state == ChannelState::Starting || state == ChannelState::Running;
}

}  // namespace pool
