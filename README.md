# Infinitas Pool Timer Controller

A firmware monorepo for a two-channel pool timer controller: an Arduino Mega 2560 remains the safety-critical controller, while an ESP32 exposes its state to Matter controllers such as Apple Home through ESP-Matter.

> **Safety warning:** This project may be installed near mains voltage and pool equipment. Disconnect all power before working on the device. Mains wiring must be performed by qualified persons in accordance with local regulations. The authors accept no responsibility for damage, injury, or loss arising from use of this project.

## Features

- Two independent timers: 30 minutes (Channel A) and 2 hours (Channel B)
- Local active-low push buttons with debouncing
- Non-blocking 16-pixel WS2812 ring animations
- Five-second LED-ring startup animation
- Safe L298N load control with enable jumpers left installed
- UART state protocol between the Mega and ESP32
- Two Matter on/off endpoints, with the Mega as the sole source of truth
- Native unit tests for protocol, timing, debounce, and LED mapping

## Architecture

```text
Local buttons + WS2812 ring + L298N loads
                    │
                    ▼
        Arduino Mega 2560 controller
        (timers, state, animations, loads)
                    │ UART: 3.3 V RX level shifted
                    ▼
          ESP32 DevKit + ESP-Matter
                    │
                    ▼
        Apple Home and other Matter controllers
```

The Mega owns timer state and all physical outputs. The ESP32 only sends line-based requests and mirrors confirmed Mega state into Matter; it never controls the L298N or LED ring.

## Supported hardware

- Arduino Mega 2560
- ESP32 DevKit (ESP32-WROOM compatible)
- Existing Infinitas L298N wiring
- 16-pixel 5 V WS2812/NeoPixel ring
- Two momentary, normally-open push buttons
- Appropriate isolated 5 V / 3.3 V power conversion for the installation

See [hardware notes](docs/hardware.md) and the complete [wiring guide](docs/wiring.md) before connecting anything.

## Wiring summary

| Function | Arduino Mega | Other end |
| --- | ---: | --- |
| Channel A | D3 = IN1, D2 = IN2 | L298N |
| Channel B | D7 = IN3, D6 = IN4 | L298N |
| Ring data | D22 | WS2812 DIN |
| Left button | D24 | Button to GND |
| Right button | D26 | Button to GND |
| Mega UART TX | D18 / TX1 | 1 kΩ then ESP32 GPIO16; 2 kΩ from GPIO16 to GND |
| Mega UART RX | D19 / RX1 | ESP32 GPIO17 / TX2 |
| Reference | GND | ESP32 GND |

Do **not** connect the Mega USB port to ESP32 USB with a normal USB cable. Both are USB devices. Use the UART above. Mega D4 and D5 are not driven because the L298N ENA/ENB jumpers remain fitted.

## Build

Install [PlatformIO Core](https://docs.platformio.org/en/latest/core/installation/index.html) for the Mega and native tests. The ESP32 gateway uses the ESP-IDF build system required by ESP-Matter.

```sh
pio run -e mega-controller
pio test -e native

# In an ESP-IDF v5.4.1 + ESP-Matter v1.4.2 shell
idf.py -C esp32-gateway set-target esp32
idf.py -C esp32-gateway build
```

## Flash

Connect one board at a time over its own USB port and select the correct serial device:

```sh
pio run -e mega-controller -t upload --upload-port /dev/tty.usbmodemXXXX
idf.py -C esp32-gateway -p /dev/tty.usbserial-XXXX flash monitor
```

Disconnect mains and pool equipment while flashing or changing low-voltage wiring.

## Matter pairing

1. Flash the ESP32 gateway and connect it to the Mega UART wiring.
2. Open the ESP-IDF serial monitor with `idf.py -C esp32-gateway monitor`.
3. ESP-Matter prints its Matter setup code and QR code during commissioning.
4. In Apple Home, choose **Add Accessory** and scan or enter that Matter setup code.
5. Name the two on/off endpoints **Pool 30 Minutes** and **Pool 2 Hours** in the controller.

Read [Matter gateway behavior](docs/matter.md) for SDK setup, reset, pairing, and resynchronization details.

## Use

- Press the left button to start the 30-minute Channel A timer; press it again while active to cancel.
- Press the right button to start the 2-hour Channel B timer; press it again while active to cancel.
- Both channels can run simultaneously.
- A load starts immediately, while its ring half fills. The countdown starts after the non-blocking startup animation completes.
- Matter endpoint state follows the state reported by the Mega, not a requested command.

## Serial protocol

Messages are ASCII lines terminated by `\n`. The ESP32 may send `START_LEFT`, `STOP_LEFT`, `START_RIGHT`, `STOP_RIGHT`, `TOGGLE_LEFT`, `TOGGLE_RIGHT`, and `STATUS`. The Mega sends `READY`, `STATE ...`, and `ERROR ...`. See the full [protocol specification](docs/protocol.md).

## Testing

The shared state machine has no Arduino dependencies and is tested on the host with `pio test -e native`. The manual hardware checklist is in [docs/testing.md](docs/testing.md).

## Repository layout

```text
src/mega-controller/    Mega firmware and physical I/O adapters
esp32-gateway/          ESP-IDF + ESP-Matter gateway
lib/pool-common/        Protocol and testable state logic
test/                   Native Unity tests
docs/                   Wiring, hardware, protocol, Matter, testing notes
.github/                CI, Dependabot, issue and PR templates
```

Contributions are welcome; read [CONTRIBUTING.md](CONTRIBUTING.md). This project is released under the [MIT License](LICENSE). It is an independent community project and is not affiliated with, endorsed by, or supported by Infinitas.

### Dependency licenses

The repository contains only this project's source. PlatformIO fetches [Adafruit NeoPixel](https://github.com/adafruit/Adafruit_NeoPixel) (LGPL-3.0-or-later) for the Mega. The gateway uses [ESP-Matter](https://github.com/espressif/esp-matter) and ESP-IDF (both Apache-2.0); no SDK is vendored. The project source remains MIT-licensed. Releases are source releases, so anyone distributing compiled firmware must also meet the applicable dependency-license obligations.
