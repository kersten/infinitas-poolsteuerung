#pragma once

#include <stdint.h>

#include "AnodeState.h"

namespace poolanode {

struct AnodeChannelConfig {
  uint32_t runtimeMs;
  uint32_t startupStepMs;
  uint8_t startupSteps;
  uint32_t startupPulseMs;
  uint32_t feedbackMs;
};

class AnodeChannel {
 public:
  explicit AnodeChannel(const AnodeChannelConfig &config);

  bool start(uint32_t now);
  bool cancel(uint32_t now);
  bool toggle(uint32_t now);
  bool update(uint32_t now);

  AnodeState state() const;
  bool outputShouldBeEnabled() const;
  uint32_t remainingRuntimeMs(uint32_t now) const;
  uint8_t startupFilledSegments(uint32_t now) const;
  bool startupPulseVisible(uint32_t now) const;
  bool feedbackVisible(uint32_t now) const;
  uint8_t elapsedSegments(uint32_t now, uint8_t segmentCount) const;
  bool activeSegmentIsGreen(uint32_t now, uint32_t blinkMs = 350) const;

 private:
  uint32_t elapsedSince(uint32_t now, uint32_t timestamp) const;
  uint32_t startupDurationMs() const;
  void transitionTo(AnodeState next, uint32_t now);

  AnodeChannelConfig config_;
  AnodeState state_;
  uint32_t stateSinceMs_;
  uint32_t runningSinceMs_;
};

}  // namespace poolanode
