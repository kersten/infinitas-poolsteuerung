#include "TimerChannel.h"

namespace pool {

TimerChannel::TimerChannel(const TimerChannelConfig &config)
    : config_(config),
      state_(ChannelState::Idle),
      stateSinceMs_(0),
      runningSinceMs_(0) {}

bool TimerChannel::start(uint32_t now) {
  if (state_ != ChannelState::Idle) {
    return false;
  }
  transitionTo(ChannelState::Starting, now);
  return true;
}

bool TimerChannel::cancel(uint32_t now) {
  if (!isChannelActive(state_)) {
    return false;
  }
  transitionTo(ChannelState::CanceledFeedback, now);
  return true;
}

bool TimerChannel::toggle(uint32_t now) {
  return isChannelActive(state_) ? cancel(now) : start(now);
}

bool TimerChannel::update(uint32_t now) {
  if (state_ == ChannelState::Starting &&
      elapsedSince(now, stateSinceMs_) >= startupDurationMs()) {
    transitionTo(ChannelState::Running, now);
    runningSinceMs_ = now;
    return true;
  }

  if (state_ == ChannelState::Running &&
      elapsedSince(now, runningSinceMs_) >= config_.durationMs) {
    transitionTo(ChannelState::FinishedFeedback, now);
    return true;
  }

  if ((state_ == ChannelState::CanceledFeedback ||
       state_ == ChannelState::FinishedFeedback) &&
      elapsedSince(now, stateSinceMs_) >= config_.feedbackMs) {
    transitionTo(ChannelState::Idle, now);
    return true;
  }

  return false;
}

ChannelState TimerChannel::state() const { return state_; }

bool TimerChannel::loadShouldBeOn() const { return isChannelActive(state_); }

uint32_t TimerChannel::remainingMs(uint32_t now) const {
  if (state_ == ChannelState::Starting) {
    return config_.durationMs;
  }
  if (state_ != ChannelState::Running) {
    return 0;
  }

  const uint32_t elapsed = elapsedSince(now, runningSinceMs_);
  return elapsed >= config_.durationMs ? 0 : config_.durationMs - elapsed;
}

uint8_t TimerChannel::startupFilledSegments(uint32_t now) const {
  if (state_ != ChannelState::Starting || config_.startupSteps == 0 ||
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

bool TimerChannel::startupPulseVisible(uint32_t now) const {
  if (state_ != ChannelState::Starting || config_.startupPulseMs == 0) {
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

bool TimerChannel::feedbackVisible(uint32_t now) const {
  if (state_ != ChannelState::CanceledFeedback &&
      state_ != ChannelState::FinishedFeedback) {
    return false;
  }
  return (elapsedSince(now, stateSinceMs_) / 150U) % 2U == 0U;
}

uint8_t TimerChannel::elapsedSegments(uint32_t now,
                                      uint8_t segmentCount) const {
  if (state_ != ChannelState::Running || segmentCount == 0 ||
      config_.durationMs == 0) {
    return 0;
  }
  const uint32_t elapsed = elapsedSince(now, runningSinceMs_);
  const uint32_t segmentMs = config_.durationMs / segmentCount;
  if (segmentMs == 0) {
    return 0;
  }
  const uint32_t segments = elapsed / segmentMs;
  return segments >= segmentCount ? segmentCount
                                  : static_cast<uint8_t>(segments);
}

bool TimerChannel::activeSegmentIsGreen(uint32_t now, uint32_t blinkMs) const {
  if (blinkMs == 0) {
    return true;
  }
  return (now / blinkMs) % 2U == 0U;
}

uint32_t TimerChannel::elapsedSince(uint32_t now, uint32_t timestamp) const {
  return now - timestamp;
}

uint32_t TimerChannel::startupDurationMs() const {
  return static_cast<uint32_t>(config_.startupSteps) * config_.startupStepMs +
         config_.startupPulseMs;
}

void TimerChannel::transitionTo(ChannelState next, uint32_t now) {
  state_ = next;
  stateSinceMs_ = now;
}

}  // namespace pool
