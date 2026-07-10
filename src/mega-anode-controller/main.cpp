#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

#include "AnodeChannel.h"
#include "ButtonDebouncer.h"
#include "LedMapper.h"
#include "Protocol.h"

namespace {

// Existing Infinitas L298N wiring. The ENA and ENB jumpers remain installed;
// D4 and D5 are inputs so firmware never fights the jumper-driven enables.
const uint8_t kSwimmingPoolIn1Pin = 3;
const uint8_t kSwimmingPoolIn2Pin = 2;
const uint8_t kWhirlpoolIn3Pin = 7;
const uint8_t kWhirlpoolIn4Pin = 6;
const uint8_t kL298EnaPin = 4;
const uint8_t kL298EnbPin = 5;

const uint8_t kLedRingDataPin = 22;
const uint8_t kSwimmingPoolButtonPin = 24;
const uint8_t kWhirlpoolButtonPin = 26;
const uint32_t kGatewayUartBaud = 115200;
const uint32_t kStateReportIntervalMs = 5000;

// Reverse either pair only if the installed L298N output polarity requires it.
const uint8_t kSwimmingPoolAnodeOnIn1Level = HIGH;
const uint8_t kSwimmingPoolAnodeOnIn2Level = LOW;
const uint8_t kWhirlpoolAnodeOnIn3Level = HIGH;
const uint8_t kWhirlpoolAnodeOnIn4Level = LOW;

const poolanode::AnodeChannelConfig kSwimmingPoolAnodeConfig = {
    30UL * 60UL * 1000UL, 100, poolanode::kLedsPerAnodeChannel, 250, 1000};
const poolanode::AnodeChannelConfig kWhirlpoolAnodeConfig = {
    2UL * 60UL * 60UL * 1000UL, 100, poolanode::kLedsPerAnodeChannel, 250,
    1000};

// Change the physical order or orientation to match the installed LED ring.
const poolanode::LedMapperConfig kLedMapperConfig = {
    {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15}, false, false};

Adafruit_NeoPixel ledRing(poolanode::kAnodeLedCount, kLedRingDataPin,
                          NEO_GRB + NEO_KHZ800);
poolanode::LedMapper ledMapper(kLedMapperConfig);
poolanode::AnodeChannel swimmingPoolAnode(kSwimmingPoolAnodeConfig);
poolanode::AnodeChannel whirlpoolAnode(kWhirlpoolAnodeConfig);
poolanode::ButtonDebouncer swimmingPoolButton;
poolanode::ButtonDebouncer whirlpoolButton;

bool swimmingPoolAnodeOutputEnabled = false;
bool whirlpoolAnodeOutputEnabled = false;
uint32_t lastStateReportMs = 0;
char gatewayLine[160];
uint8_t gatewayLineLength = 0;

uint32_t nowMs() { return static_cast<uint32_t>(millis()); }

void enableSwimmingPoolAnodeOutput() {
  digitalWrite(kSwimmingPoolIn1Pin, kSwimmingPoolAnodeOnIn1Level);
  digitalWrite(kSwimmingPoolIn2Pin, kSwimmingPoolAnodeOnIn2Level);
}

void disableSwimmingPoolAnodeOutput() {
  digitalWrite(kSwimmingPoolIn1Pin, LOW);
  digitalWrite(kSwimmingPoolIn2Pin, LOW);
}

void enableWhirlpoolAnodeOutput() {
  digitalWrite(kWhirlpoolIn3Pin, kWhirlpoolAnodeOnIn3Level);
  digitalWrite(kWhirlpoolIn4Pin, kWhirlpoolAnodeOnIn4Level);
}

void disableWhirlpoolAnodeOutput() {
  digitalWrite(kWhirlpoolIn3Pin, LOW);
  digitalWrite(kWhirlpoolIn4Pin, LOW);
}

void applyAnodeOutputStates() {
  const bool swimmingPoolShouldBeEnabled =
      swimmingPoolAnode.outputShouldBeEnabled();
  const bool whirlpoolShouldBeEnabled = whirlpoolAnode.outputShouldBeEnabled();
  if (swimmingPoolShouldBeEnabled != swimmingPoolAnodeOutputEnabled) {
    if (swimmingPoolShouldBeEnabled) {
      enableSwimmingPoolAnodeOutput();
    } else {
      disableSwimmingPoolAnodeOutput();
    }
    swimmingPoolAnodeOutputEnabled = swimmingPoolShouldBeEnabled;
  }
  if (whirlpoolShouldBeEnabled != whirlpoolAnodeOutputEnabled) {
    if (whirlpoolShouldBeEnabled) {
      enableWhirlpoolAnodeOutput();
    } else {
      disableWhirlpoolAnodeOutput();
    }
    whirlpoolAnodeOutputEnabled = whirlpoolShouldBeEnabled;
  }
}

void sendGatewayLine(const char *line) { Serial1.println(line); }

void sendState(uint32_t now) {
  const poolanode::protocol::StateSnapshot state = {
      swimmingPoolAnode.state(), whirlpoolAnode.state(),
      swimmingPoolAnode.remainingRuntimeMs(now),
      whirlpoolAnode.remainingRuntimeMs(now)};
  char message[180];
  if (poolanode::protocol::serializeState(message, sizeof(message), state)) {
    sendGatewayLine(message);
  }
  lastStateReportMs = now;
}

void sendUnknownCommandError() {
  char message[128];
  if (poolanode::protocol::serializeError(message, sizeof(message),
                                          "UNKNOWN_COMMAND",
                                          "unsupported_command")) {
    sendGatewayLine(message);
  }
}

bool processGatewayCommand(poolanode::protocol::Command command,
                           uint32_t now) {
  switch (command) {
    case poolanode::protocol::Command::StartSwimmingPool:
      return swimmingPoolAnode.start(now);
    case poolanode::protocol::Command::StopSwimmingPool:
      return swimmingPoolAnode.cancel(now);
    case poolanode::protocol::Command::StartWhirlpool:
      return whirlpoolAnode.start(now);
    case poolanode::protocol::Command::StopWhirlpool:
      return whirlpoolAnode.cancel(now);
    case poolanode::protocol::Command::ToggleSwimmingPool:
      return swimmingPoolAnode.toggle(now);
    case poolanode::protocol::Command::ToggleWhirlpool:
      return whirlpoolAnode.toggle(now);
    case poolanode::protocol::Command::Status:
      sendState(now);
      return false;
    case poolanode::protocol::Command::Unknown:
      return false;
  }
  return false;
}

void handleGatewayLine(const char *line, uint32_t now) {
  const poolanode::protocol::ParsedCommand parsed =
      poolanode::protocol::parseCommand(line);
  if (!parsed.valid) {
    sendUnknownCommandError();
    return;
  }
  (void)processGatewayCommand(parsed.command, now);
  if (parsed.command != poolanode::protocol::Command::Status) {
    applyAnodeOutputStates();
    // A state event is the command acknowledgement; the gateway never guesses.
    sendState(now);
  }
}

void pollGatewayUart(uint32_t now) {
  while (Serial1.available() > 0) {
    const char character = static_cast<char>(Serial1.read());
    if (character == '\r') {
      continue;
    }
    if (character == '\n') {
      gatewayLine[gatewayLineLength] = '\0';
      if (gatewayLineLength > 0) {
        handleGatewayLine(gatewayLine, now);
      }
      gatewayLineLength = 0;
    } else if (gatewayLineLength < sizeof(gatewayLine) - 1) {
      gatewayLine[gatewayLineLength++] = character;
    } else {
      gatewayLineLength = 0;
      sendUnknownCommandError();
    }
  }
}

uint32_t color(uint8_t red, uint8_t green, uint8_t blue) {
  return ledRing.Color(red, green, blue);
}

void setSegment(poolanode::AnodeChannelId channel, uint8_t segment,
                uint32_t pixelColor) {
  ledRing.setPixelColor(ledMapper.physicalIndex(channel, segment), pixelColor);
}

void clearAnodeHalf(poolanode::AnodeChannelId channel) {
  for (uint8_t segment = 0; segment < poolanode::kLedsPerAnodeChannel;
       ++segment) {
    setSegment(channel, segment, 0);
  }
}

void fillAnodeHalf(poolanode::AnodeChannelId channel, uint32_t pixelColor) {
  for (uint8_t segment = 0; segment < poolanode::kLedsPerAnodeChannel;
       ++segment) {
    setSegment(channel, segment, pixelColor);
  }
}

void renderAnodeChannel(poolanode::AnodeChannelId channel,
                        const poolanode::AnodeChannel &anode,
                        uint32_t now) {
  clearAnodeHalf(channel);
  const poolanode::AnodeState state = anode.state();
  if (state == poolanode::AnodeState::Starting) {
    if (anode.startupPulseVisible(now)) {
      fillAnodeHalf(channel, color(255, 75, 0));
    } else {
      const uint8_t filled = anode.startupFilledSegments(now);
      for (uint8_t segment = 0; segment < filled; ++segment) {
        setSegment(channel, segment, color(255, 50, 0));
      }
    }
    return;
  }
  if (state == poolanode::AnodeState::Running) {
    const uint8_t elapsed =
        anode.elapsedSegments(now, poolanode::kLedsPerAnodeChannel);
    for (uint8_t segment = 0; segment < poolanode::kLedsPerAnodeChannel;
         ++segment) {
      if (segment < elapsed) {
        setSegment(channel, segment, color(0, 180, 0));
      } else if (segment == elapsed) {
        setSegment(channel, segment,
                   anode.activeSegmentIsGreen(now) ? color(0, 180, 0)
                                                   : color(220, 0, 0));
      } else {
        setSegment(channel, segment, color(220, 0, 0));
      }
    }
    return;
  }
  if (state == poolanode::AnodeState::CanceledFeedback &&
      anode.feedbackVisible(now)) {
    fillAnodeHalf(channel, color(255, 170, 0));
  } else if (state == poolanode::AnodeState::FinishedFeedback &&
             anode.feedbackVisible(now)) {
    fillAnodeHalf(channel, color(0, 220, 0));
  }
}

void renderLedRing(uint32_t now) {
  renderAnodeChannel(poolanode::AnodeChannelId::SwimmingPool,
                     swimmingPoolAnode, now);
  renderAnodeChannel(poolanode::AnodeChannelId::Whirlpool, whirlpoolAnode,
                     now);
  ledRing.show();
}

}  // namespace

void setup() {
  pinMode(kSwimmingPoolIn1Pin, OUTPUT);
  pinMode(kSwimmingPoolIn2Pin, OUTPUT);
  pinMode(kWhirlpoolIn3Pin, OUTPUT);
  pinMode(kWhirlpoolIn4Pin, OUTPUT);
  pinMode(kL298EnaPin, INPUT);
  pinMode(kL298EnbPin, INPUT);
  disableSwimmingPoolAnodeOutput();
  disableWhirlpoolAnodeOutput();

  pinMode(kSwimmingPoolButtonPin, INPUT_PULLUP);
  pinMode(kWhirlpoolButtonPin, INPUT_PULLUP);
  ledRing.begin();
  ledRing.clear();
  ledRing.show();

  Serial.begin(115200);
  Serial1.begin(kGatewayUartBaud);
  sendGatewayLine("READY");
  sendState(nowMs());
}

void loop() {
  const uint32_t now = nowMs();
  bool stateChanged = false;

  pollGatewayUart(now);
  if (swimmingPoolButton.poll(digitalRead(kSwimmingPoolButtonPin) == HIGH,
                              now)) {
    stateChanged = swimmingPoolAnode.toggle(now) || stateChanged;
  }
  if (whirlpoolButton.poll(digitalRead(kWhirlpoolButtonPin) == HIGH, now)) {
    stateChanged = whirlpoolAnode.toggle(now) || stateChanged;
  }
  stateChanged = swimmingPoolAnode.update(now) || stateChanged;
  stateChanged = whirlpoolAnode.update(now) || stateChanged;

  applyAnodeOutputStates();
  renderLedRing(now);

  const bool anAnodeChannelIsActive =
      swimmingPoolAnode.outputShouldBeEnabled() ||
      whirlpoolAnode.outputShouldBeEnabled();
  if (stateChanged ||
      (anAnodeChannelIsActive &&
       static_cast<uint32_t>(now - lastStateReportMs) >=
           kStateReportIntervalMs)) {
    sendState(now);
  }
}
