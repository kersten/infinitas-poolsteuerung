#include <Arduino.h>
#include <Matter.h>

#include "AnodeState.h"
#include "Protocol.h"

namespace {

const uint32_t kMegaUartBaud = 115200;
const int8_t kMegaUartRxPin = 16;
const int8_t kMegaUartTxPin = 17;
const uint32_t kCommissioningReminderIntervalMs = 10000;
const char kSwimmingPoolAnodeMatterName[] = "Swimming Pool Anode";
const char kWhirlpoolAnodeMatterName[] = "Whirlpool Anode";

MatterOnOffPlugin swimmingPoolAnodeSwitch;
MatterOnOffPlugin whirlpoolAnodeSwitch;

bool applyingMegaState = false;
char megaLine[192];
uint8_t megaLineLength = 0;
uint32_t lastCommissioningReminderMs = 0;

void sendCommand(poolanode::protocol::Command command) {
  Serial2.print(poolanode::protocol::commandName(command));
  Serial2.print('\n');
}

void requestStatus() { sendCommand(poolanode::protocol::Command::Status); }

void publishConfirmedSwitchState(MatterOnOffPlugin &matterSwitch, bool on) {
  if (matterSwitch.getOnOff() == on) {
    return;
  }
  applyingMegaState = true;
  matterSwitch.setOnOff(on);
  applyingMegaState = false;
}

void applyMegaState(const poolanode::protocol::StateSnapshot &state) {
  publishConfirmedSwitchState(
      swimmingPoolAnodeSwitch,
      poolanode::isAnodeChannelActive(state.swimmingPool));
  publishConfirmedSwitchState(whirlpoolAnodeSwitch,
                              poolanode::isAnodeChannelActive(state.whirlpool));
}

bool requestSwimmingPoolAnodeState(bool on) {
  if (applyingMegaState) {
    return true;
  }
  sendCommand(on ? poolanode::protocol::Command::StartSwimmingPool
                 : poolanode::protocol::Command::StopSwimmingPool);
  // The Mega's STATE event is the acknowledgement. Returning false keeps the
  // Matter value at its last confirmed state until that event arrives.
  return false;
}

bool requestWhirlpoolAnodeState(bool on) {
  if (applyingMegaState) {
    return true;
  }
  sendCommand(on ? poolanode::protocol::Command::StartWhirlpool
                 : poolanode::protocol::Command::StopWhirlpool);
  return false;
}

void handleMegaLine(const char *line) {
  if (strcmp(line, "READY") == 0) {
    requestStatus();
    return;
  }
  poolanode::protocol::StateSnapshot state;
  if (poolanode::protocol::parseState(line, state)) {
    applyMegaState(state);
    return;
  }
  Serial.print("Mega UART: ");
  Serial.println(line);
}

void pollMegaUart() {
  while (Serial2.available() > 0) {
    const char character = static_cast<char>(Serial2.read());
    if (character == '\r') {
      continue;
    }
    if (character == '\n') {
      megaLine[megaLineLength] = '\0';
      if (megaLineLength > 0) {
        handleMegaLine(megaLine);
      }
      megaLineLength = 0;
    } else if (megaLineLength < sizeof(megaLine) - 1) {
      megaLine[megaLineLength++] = character;
    } else {
      megaLineLength = 0;
    }
  }
}

void printCommissioningInformation(uint32_t now) {
  if (Matter.isDeviceCommissioned() ||
      static_cast<uint32_t>(now - lastCommissioningReminderMs) <
          kCommissioningReminderIntervalMs) {
    return;
  }
  lastCommissioningReminderMs = now;
  Serial.println("Matter is ready for commissioning.");
  Serial.print("Manual pairing code: ");
  Serial.println(Matter.getManualPairingCode());
  Serial.print("QR code URL: ");
  Serial.println(Matter.getOnboardingQRCodeUrl());
}

}  // namespace

void setup() {
  Serial.begin(115200);
  Serial2.begin(kMegaUartBaud, SERIAL_8N1, kMegaUartRxPin, kMegaUartTxPin);

  swimmingPoolAnodeSwitch.begin(false);
  swimmingPoolAnodeSwitch.onChange(requestSwimmingPoolAnodeState);
  whirlpoolAnodeSwitch.begin(false);
  whirlpoolAnodeSwitch.onChange(requestWhirlpoolAnodeState);
  Matter.begin();

  Serial.print("Matter switch: ");
  Serial.println(kSwimmingPoolAnodeMatterName);
  Serial.print("Matter switch: ");
  Serial.println(kWhirlpoolAnodeMatterName);
  requestStatus();
}

void loop() {
  pollMegaUart();
  printCommissioningInformation(static_cast<uint32_t>(millis()));
}
