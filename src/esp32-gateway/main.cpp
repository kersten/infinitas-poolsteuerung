#include <Arduino.h>
#include <HomeSpan.h>
#include <string.h>

#include "Protocol.h"

namespace {

const uint32_t kUartBaud = 115200;
const int kEsp32UartRxPin = 16;
const int kEsp32UartTxPin = 17;
HardwareSerial controllerUart(2);

class PoolSwitch : public Service::Switch {
 public:
  explicit PoolSwitch(pool::protocol::Command command)
      : Service::Switch(),
        on_(new Characteristic::On(false)),
        command_(command) {}

  boolean update() override {
    const bool requestedOn = on_->getNewVal();
    const pool::protocol::Command command =
        requestedOn ? command_
                    : (command_ == pool::protocol::Command::StartLeft
                           ? pool::protocol::Command::StopLeft
                           : pool::protocol::Command::StopRight);
    controllerUart.println(pool::protocol::commandName(command));

    // Reject the optimistic HomeKit value. The Arduino's next STATE event is
    // the sole acknowledgement and will update this characteristic if accepted.
    return false;
  }

  void setFromController(bool on) {
    if (on_->getVal() != on) {
      on_->setVal(on);
    }
  }

 private:
  SpanCharacteristic *on_;
  pool::protocol::Command command_;
};

PoolSwitch *leftSwitch = 0;
PoolSwitch *rightSwitch = 0;
char uartLine[160];
uint8_t uartLineLength = 0;

void requestStatus() { controllerUart.println("STATUS"); }

void applyState(const pool::protocol::StateSnapshot &state) {
  if (leftSwitch != 0) {
    leftSwitch->setFromController(pool::isChannelActive(state.left));
  }
  if (rightSwitch != 0) {
    rightSwitch->setFromController(pool::isChannelActive(state.right));
  }
}

void handleControllerLine(const char *line) {
  if (strcmp(line, "READY") == 0) {
    requestStatus();
    return;
  }

  pool::protocol::StateSnapshot state;
  if (pool::protocol::parseState(line, state)) {
    applyState(state);
    return;
  }

  // Unknown/malformed input cannot change HomeKit state. Serial remains useful
  // for diagnostics when the gateway is connected to a computer.
  Serial.print("Controller UART: ");
  Serial.println(line);
}

void pollControllerUart() {
  while (controllerUart.available() > 0) {
    const char character = static_cast<char>(controllerUart.read());
    if (character == '\r') {
      continue;
    }
    if (character == '\n') {
      uartLine[uartLineLength] = '\0';
      if (uartLineLength > 0) {
        handleControllerLine(uartLine);
      }
      uartLineLength = 0;
      continue;
    }
    if (uartLineLength < sizeof(uartLine) - 1) {
      uartLine[uartLineLength++] = character;
    } else {
      uartLineLength = 0;  // Discard overlong line without changing state.
    }
  }
}

void addAccessoryInformation(const char *name, const char *serial) {
  new Service::AccessoryInformation();
  new Characteristic::Identify();
  new Characteristic::Name(name);
  new Characteristic::Manufacturer("Infinitas Pool Timer Community");
  new Characteristic::SerialNumber(serial);
  new Characteristic::Model("UART HomeKit Gateway");
  new Characteristic::FirmwareRevision("1.0.0");
}

}  // namespace

void setup() {
  Serial.begin(115200);
  controllerUart.begin(kUartBaud, SERIAL_8N1, kEsp32UartRxPin, kEsp32UartTxPin);

  homeSpan.begin(Category::Bridges, "Pool Timer Gateway");

  // The bridge itself exposes mandatory HAP bridge services.
  new SpanAccessory();
  addAccessoryInformation("Pool Timer Gateway", "pool-gateway-001");
  new Service::HAPProtocolInformation();
  new Characteristic::Version("1.1.0");

  new SpanAccessory();
  addAccessoryInformation("Pool 30 Minutes", "pool-left-001");
  leftSwitch = new PoolSwitch(pool::protocol::Command::StartLeft);

  new SpanAccessory();
  addAccessoryInformation("Pool 2 Hours", "pool-right-001");
  rightSwitch = new PoolSwitch(pool::protocol::Command::StartRight);

  // Request state even if the Mega booted before this gateway.
  requestStatus();
}

void loop() {
  homeSpan.poll();
  pollControllerUart();
}
