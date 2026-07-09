#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

#include "ButtonDebouncer.h"
#include "LedMapper.h"
#include "Protocol.h"
#include "TimerChannel.h"

namespace {

// Existing Infinitas L298N wiring. ENA (D4) and ENB (D5) retain their L298N
// jumpers and are intentionally never driven by firmware.
const uint8_t kL298In1Pin = 3;
const uint8_t kL298In2Pin = 2;
const uint8_t kL298In3Pin = 7;
const uint8_t kL298In4Pin = 6;
const uint8_t kL298EnaPin = 4;
const uint8_t kL298EnbPin = 5;

const uint8_t kRingPin = 22;
const uint8_t kLeftButtonPin = 24;
const uint8_t kRightButtonPin = 26;
const uint32_t kUartBaud = 115200;
const uint32_t kStatusIntervalMs = 5000;

const pool::TimerChannelConfig kLeftTimerConfig = {
    30UL * 60UL * 1000UL,  // 30 minutes
    100,                   // startup fill step interval
    pool::kLedsPerChannel,
    250,                   // startup pulse
    1000,                  // cancel / finish feedback
};
const pool::TimerChannelConfig kRightTimerConfig = {
    2UL * 60UL * 60UL * 1000UL,  // 2 hours
    100,
    pool::kLedsPerChannel,
    250,
    1000,
};

// Change this table if the physical ring starts at a different pixel or has a
// different cable/order arrangement. The two booleans reverse each half.
const pool::LedMapperConfig kLedMapConfig = {
    {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15}, false, false};

Adafruit_NeoPixel ring(pool::kLedCount, kRingPin, NEO_GRB + NEO_KHZ800);
pool::LedMapper ledMapper(kLedMapConfig);
pool::TimerChannel leftTimer(kLeftTimerConfig);
pool::TimerChannel rightTimer(kRightTimerConfig);
pool::ButtonDebouncer leftButton;
pool::ButtonDebouncer rightButton;

bool leftLoadOn = false;
bool rightLoadOn = false;
uint32_t lastStatusMs = 0;
char uartLine[128];
uint8_t uartLineLength = 0;

uint32_t nowMs() { return static_cast<uint32_t>(millis()); }

void setLeftLoad(bool on) {
  digitalWrite(kL298In1Pin, on ? HIGH : LOW);
  digitalWrite(kL298In2Pin, LOW);
}

void setRightLoad(bool on) {
  digitalWrite(kL298In3Pin, on ? HIGH : LOW);
  digitalWrite(kL298In4Pin, LOW);
}

void applyLoadStates() {
  const bool nextLeft = leftTimer.loadShouldBeOn();
  const bool nextRight = rightTimer.loadShouldBeOn();
  if (nextLeft != leftLoadOn) {
    setLeftLoad(nextLeft);
    leftLoadOn = nextLeft;
  }
  if (nextRight != rightLoadOn) {
    setRightLoad(nextRight);
    rightLoadOn = nextRight;
  }
}

void sendLine(const char *line) { Serial1.println(line); }

void sendState(uint32_t now) {
  const pool::protocol::StateSnapshot state = {
      leftTimer.state(), rightTimer.state(), leftTimer.remainingMs(now),
      rightTimer.remainingMs(now)};
  char message[144];
  if (pool::protocol::serializeState(message, sizeof(message), state)) {
    sendLine(message);
  }
  lastStatusMs = now;
}

void sendUnknownCommandError() {
  char message[128];
  if (pool::protocol::serializeError(message, sizeof(message), "UNKNOWN_COMMAND",
                                     "unsupported_command")) {
    sendLine(message);
  }
}

bool processCommand(pool::protocol::Command command, uint32_t now) {
  switch (command) {
    case pool::protocol::Command::StartLeft:
      return leftTimer.start(now);
    case pool::protocol::Command::StopLeft:
      return leftTimer.cancel(now);
    case pool::protocol::Command::StartRight:
      return rightTimer.start(now);
    case pool::protocol::Command::StopRight:
      return rightTimer.cancel(now);
    case pool::protocol::Command::ToggleLeft:
      return leftTimer.toggle(now);
    case pool::protocol::Command::ToggleRight:
      return rightTimer.toggle(now);
    case pool::protocol::Command::Status:
      sendState(now);
      return false;
    case pool::protocol::Command::Unknown:
      break;
  }
  return false;
}

void handleCommandLine(const char *line, uint32_t now) {
  const pool::protocol::ParsedCommand parsed = pool::protocol::parseCommand(line);
  if (!parsed.valid) {
    sendUnknownCommandError();
    return;
  }

  const bool changed = processCommand(parsed.command, now);
  // A state response is the acknowledgement. This is also sent for no-op start
  // or stop requests so the gateway can always reconcile with Arduino truth.
  if (parsed.command != pool::protocol::Command::Status) {
    applyLoadStates();
    sendState(now);
  }
  (void)changed;
}

void pollUart(uint32_t now) {
  while (Serial1.available() > 0) {
    const char character = static_cast<char>(Serial1.read());
    if (character == '\r') {
      continue;
    }
    if (character == '\n') {
      uartLine[uartLineLength] = '\0';
      if (uartLineLength > 0) {
        handleCommandLine(uartLine, now);
      }
      uartLineLength = 0;
      continue;
    }
    if (uartLineLength < sizeof(uartLine) - 1) {
      uartLine[uartLineLength++] = character;
    } else {
      uartLineLength = 0;
      sendUnknownCommandError();
    }
  }
}

uint32_t color(uint8_t red, uint8_t green, uint8_t blue) {
  return ring.Color(red, green, blue);
}

void setSegment(pool::ChannelId channel, uint8_t segment, uint32_t pixelColor) {
  ring.setPixelColor(ledMapper.physicalIndex(channel, segment), pixelColor);
}

void clearHalf(pool::ChannelId channel) {
  for (uint8_t segment = 0; segment < pool::kLedsPerChannel; ++segment) {
    setSegment(channel, segment, 0);
  }
}

void fillHalf(pool::ChannelId channel, uint32_t pixelColor) {
  for (uint8_t segment = 0; segment < pool::kLedsPerChannel; ++segment) {
    setSegment(channel, segment, pixelColor);
  }
}

void renderTimer(pool::ChannelId channel, const pool::TimerChannel &timer,
                 uint32_t now) {
  clearHalf(channel);
  const pool::ChannelState state = timer.state();
  if (state == pool::ChannelState::Starting) {
    if (timer.startupPulseVisible(now)) {
      fillHalf(channel, color(255, 75, 0));
    } else {
      const uint8_t filled = timer.startupFilledSegments(now);
      for (uint8_t segment = 0; segment < filled; ++segment) {
        setSegment(channel, segment, color(255, 50, 0));
      }
    }
    return;
  }

  if (state == pool::ChannelState::Running) {
    const uint8_t elapsed = timer.elapsedSegments(now, pool::kLedsPerChannel);
    for (uint8_t segment = 0; segment < pool::kLedsPerChannel; ++segment) {
      if (segment < elapsed) {
        setSegment(channel, segment, color(0, 180, 0));
      } else if (segment == elapsed) {
        setSegment(channel, segment,
                   timer.activeSegmentIsGreen(now) ? color(0, 180, 0)
                                                  : color(220, 0, 0));
      } else {
        setSegment(channel, segment, color(220, 0, 0));
      }
    }
    return;
  }

  if (state == pool::ChannelState::CanceledFeedback && timer.feedbackVisible(now)) {
    fillHalf(channel, color(255, 170, 0));
  } else if (state == pool::ChannelState::FinishedFeedback &&
             timer.feedbackVisible(now)) {
    fillHalf(channel, color(0, 220, 0));
  }
}

void renderLeds(uint32_t now) {
  renderTimer(pool::ChannelId::Left, leftTimer, now);
  renderTimer(pool::ChannelId::Right, rightTimer, now);
  ring.show();
}

}  // namespace

void setup() {
  // Safe boot state: both loads are off before UART or buttons are processed.
  pinMode(kL298In1Pin, OUTPUT);
  pinMode(kL298In2Pin, OUTPUT);
  pinMode(kL298In3Pin, OUTPUT);
  pinMode(kL298In4Pin, OUTPUT);
  pinMode(kL298EnaPin, INPUT);  // Never OUTPUT: L298N ENA jumper stays fitted.
  pinMode(kL298EnbPin, INPUT);  // Never OUTPUT: L298N ENB jumper stays fitted.
  setLeftLoad(false);
  setRightLoad(false);

  pinMode(kLeftButtonPin, INPUT_PULLUP);
  pinMode(kRightButtonPin, INPUT_PULLUP);
  ring.begin();
  ring.clear();
  ring.show();

  Serial.begin(115200);
  Serial1.begin(kUartBaud);
  sendLine("READY");
  sendState(nowMs());
}

void loop() {
  const uint32_t now = nowMs();
  bool stateChanged = false;

  pollUart(now);
  if (leftButton.poll(digitalRead(kLeftButtonPin) == HIGH, now)) {
    stateChanged = leftTimer.toggle(now) || stateChanged;
  }
  if (rightButton.poll(digitalRead(kRightButtonPin) == HIGH, now)) {
    stateChanged = rightTimer.toggle(now) || stateChanged;
  }
  stateChanged = leftTimer.update(now) || stateChanged;
  stateChanged = rightTimer.update(now) || stateChanged;

  applyLoadStates();
  renderLeds(now);

  const bool aTimerIsActive = leftTimer.loadShouldBeOn() || rightTimer.loadShouldBeOn();
  if (stateChanged ||
      (aTimerIsActive && static_cast<uint32_t>(now - lastStatusMs) >=
                              kStatusIntervalMs)) {
    sendState(now);
  }
}
