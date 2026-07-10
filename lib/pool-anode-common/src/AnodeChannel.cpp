#include "AnodeChannel.h"

namespace poolanode {

AnodeChannel::AnodeChannel(const AnodeChannelConfig &config)
    : config_(config),
      state_(AnodeState::Idle),
      stateSinceMs_(0),
      runningSinceMs_(0) {}

bool AnodeChannel::start(uint32_t now) {
  if (state_ != AnodeState::Idle) {
    return false;
  }
  transitionTo(AnodeState::Starting, now);
  return true;
}

bool AnodeChannel::cancel(uint32_t now) {
  if (!isAnodeChannelActive(state_)) {
    return false;
  }
  transitionTo(AnodeState::CanceledFeedback, now);
  return true;
}

bool AnodeChannel::toggle(uint32_t now) {
  return isAnodeChannelActive(state_) ? cancel(now) : start(now);
}

bool AnodeChannel::update(uint32_t now) {
  if (state_ == AnodeState::Starting &&
      elapsedSince(now, stateSinceMs_) >= startupDurationMs()) {
    transitionTo(AnodeState::Running, now);
    runningSinceMs_ = now;
    return true;
  }
  if (state_ == AnodeState::Running &&
      elapsedSince(now, runningSinceMs_) >= config_.runtimeMs) {
    transitionTo(AnodeState::FinishedFeedback, now);
    return true;
  }
  if ((state_ == AnodeState::CanceledFeedback ||
       state_ == AnodeState::FinishedFeedback) &&
      elapsedSince(now, stateSinceMs_) >= config_.feedbackMs) {
    transitionTo(AnodeState::Idle, now);
    return true;
  }
  return false;
}

AnodeState AnodeChannel::state() const { return state_; }

bool AnodeChannel::outputShouldBeEnabled() const {
  return isAnodeChannelActive(state_);
}

uint32_t AnodeChannel::remainingRuntimeMs(uint32_t now) const {
  if (state_ == AnodeState::Starting) {
    return config_.runtimeMs;
  }
  if (state_ != AnodeState::Running) {
    return 0;
  }
  const uint32_t elapsed = elapsedSince(now, runningSinceMs_);
  return elapsed >= config_.runtimeMs ? 0 : config_.runtimeMs - elapsed;
}

uint8_t AnodeChannel::startupFilledSegments(uint32_t now) const {
  if (state_ != AnodeState::Starting || config_.startupSteps == 0 ||
      config_.startupStepMs == 0) {
    return 0;
  }
  const uint32_t elapsed = elapsedSince(now, stateSinceMs_);
  const uint32_t fillDuration =
      static_cast<uint32_t>(config_.startupSteps) * config_.startupStepMs;
  if (elapsed >= fillDuration) {
    return config_.startupSteps;
  }
  const uint32_t filled = elapsed / config_.startupStepMs + 1;
  return filled > config_.startupSteps ? config_.startupSteps
                                       : static_cast<uint8_t>(filled);
}

bool AnodeChannel::startupPulseVisible(uint32_t now) const {
  if (state_ != AnodeState::Starting || config_.startupPulseMs == 0) {
    return false;
  }
  const uint32_t elapsed = elapsedSince(now, stateSinceMs_);
  const uint32_t fillDuration =
      static_cast<uint32_t>(config_.startupSteps) * config_.startupStepMs;
  if (elapsed < fillDuration || elapsed >= startupDurationMs()) {
    return false;
  }
  return ((elapsed - fillDuration) / 75U) % 2U == 0U;
}

bool AnodeChannel::feedbackVisible(uint32_t now) const {
  if (state_ != AnodeState::CanceledFeedback &&
      state_ != AnodeState::FinishedFeedback) {
    return false;
  }
  return (elapsedSince(now, stateSinceMs_) / 150U) % 2U == 0U;
}

uint8_t AnodeChannel::elapsedSegments(uint32_t now,
                                      uint8_t segmentCount) const {
  if (state_ != AnodeState::Running || segmentCount == 0 ||
      config_.runtimeMs == 0) {
    return 0;
  }
  const uint32_t segmentMs = config_.runtimeMs / segmentCount;
  if (segmentMs == 0) {
    return 0;
  }
  const uint32_t elapsed = elapsedSince(now, runningSinceMs_);
  const uint32_t segments = elapsed / segmentMs;
  return segments >= segmentCount ? segmentCount
                                  : static_cast<uint8_t>(segments);
}

bool AnodeChannel::activeSegmentIsGreen(uint32_t now,
                                        uint32_t blinkMs) const {
  return blinkMs == 0 || (now / blinkMs) % 2U == 0U;
}

uint32_t AnodeChannel::elapsedSince(uint32_t now, uint32_t timestamp) const {
  return now - timestamp;
}

uint32_t AnodeChannel::startupDurationMs() const {
  return static_cast<uint32_t>(config_.startupSteps) * config_.startupStepMs +
         config_.startupPulseMs;
}

void AnodeChannel::transitionTo(AnodeState next, uint32_t now) {
  state_ = next;
  stateSinceMs_ = now;
}

}  // namespace poolanode
