#pragma once

#include <stdint.h>
#include <string.h>

namespace poolanode {

enum class AnodeChannelId : uint8_t {
  SwimmingPool,
  Whirlpool,
};

enum class AnodeState : uint8_t {
  Idle,
  Starting,
  Running,
  CanceledFeedback,
  FinishedFeedback,
};

inline const char *anodeStateName(AnodeState state) {
  switch (state) {
    case AnodeState::Idle:
      return "IDLE";
    case AnodeState::Starting:
      return "STARTING";
    case AnodeState::Running:
      return "RUNNING";
    case AnodeState::CanceledFeedback:
      return "CANCELED_FEEDBACK";
    case AnodeState::FinishedFeedback:
      return "FINISHED_FEEDBACK";
  }
  return "IDLE";
}

inline bool parseAnodeState(const char *text, AnodeState &state) {
  if (text == 0) {
    return false;
  }
  if (strcmp(text, "IDLE") == 0) {
    state = AnodeState::Idle;
  } else if (strcmp(text, "STARTING") == 0) {
    state = AnodeState::Starting;
  } else if (strcmp(text, "RUNNING") == 0) {
    state = AnodeState::Running;
  } else if (strcmp(text, "CANCELED_FEEDBACK") == 0) {
    state = AnodeState::CanceledFeedback;
  } else if (strcmp(text, "FINISHED_FEEDBACK") == 0) {
    state = AnodeState::FinishedFeedback;
  } else {
    return false;
  }
  return true;
}

inline bool isAnodeChannelActive(AnodeState state) {
  return state == AnodeState::Starting || state == AnodeState::Running;
}

}  // namespace poolanode
