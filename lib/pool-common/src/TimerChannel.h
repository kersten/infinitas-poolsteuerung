#pragma once

#include <stdint.h>

#include "ChannelState.h"

namespace pool {

struct TimerChannelConfig {
  uint32_t durationMs;
  uint32_t startupStepMs;
  uint8_t startupSteps;
  uint32_t startupPulseMs;
  uint32_t feedbackMs;
};

// A hardware-independent, non-blocking timer state machine. All timestamps are
// uint32_t millis() values; subtraction is deliberately used for overflow safety.
class TimerChannel {
 public:
  explicit TimerChannel(const TimerChannelConfig &config);

  bool start(uint32_t now);
  bool cancel(uint32_t now);
  bool toggle(uint32_t now);
  bool update(uint32_t now);

  ChannelState state() const;
  bool loadShouldBeOn() const;
  uint32_t remainingMs(uint32_t now) const;
  uint8_t startupFilledSegments(uint32_t now) const;
  bool startupPulseVisible(uint32_t now) const;
  bool feedbackVisible(uint32_t now) const;
  uint8_t elapsedSegments(uint32_t now, uint8_t segmentCount) const;
  bool activeSegmentIsGreen(uint32_t now, uint32_t blinkMs = 350) const;

 private:
  uint32_t elapsedSince(uint32_t now, uint32_t timestamp) const;
  uint32_t startupDurationMs() const;
  void transitionTo(ChannelState next, uint32_t now);

  TimerChannelConfig config_;
  ChannelState state_;
  uint32_t stateSinceMs_;
  uint32_t runningSinceMs_;
};

}  // namespace pool
