# pool-anode-controller

Dual-channel copper anode timer controller for swimming pool and whirlpool
ionization systems.

> **Safety warning:** This DIY firmware controls copper anodes near pool
> equipment. Disconnect power before opening, wiring, flashing, or servicing
> the installation. Qualified persons must handle mains wiring under local
> regulations. Test copper, pH, alkalinity, and every other relevant water
> parameter with suitable pool-test equipment. The firmware controls anode
> runtime only: it does not measure copper concentration and cannot guarantee
> safe or correct water chemistry. The authors are not responsible for damage,
> unsafe water conditions, staining, corrosion, injury, or equipment failure.

## Features

- Independent Swimming Pool Anode (2 hours) and Whirlpool Anode (30 minutes)
  channels that can run together
- Local active-low buttons with software debounce
- Non-blocking 16-pixel WS2812/NeoPixel animations: startup, runtime, cancel,
  and finish feedback
- Arduino Mega is the authoritative anode-runtime and physical-output
  controller
- ESP32 Matter gateway mirrors only Mega-confirmed anode state over UART
- Native PlatformIO tests for the protocol, channel state machine, buttons,
  LED mapping, remaining runtime, and `millis()` overflow behavior

## Architecture

```text
Swimming Pool button ─┐
Whirlpool button ─────┼──> Arduino Mega 2560 ──> L298N IN1/IN2: Swimming Pool Anode
WS2812 ring ──────────┘        │                └> L298N IN3/IN4: Whirlpool Anode
                               │ Serial1 UART (Mega state is authoritative)
                               ▼
                         ESP32 Matter gateway ──> Matter controllers
```

The ESP32 does not control the copper anode outputs, the LED ring, or anode
runtime. It sends UART requests and applies a Matter state only after an
Arduino `STATE` event confirms it.

## Supported hardware

- Arduino Mega 2560
- ESP32 DevKit / ESP32-WROOM-compatible board with UART2 on GPIO16/GPIO17
- Existing Infinitas L298N wiring
- 16-pixel, 5 V WS2812/NeoPixel ring
- Two normally-open momentary buttons
- Properly rated low-voltage supply and a Mega-to-ESP32 UART level shifter

Read [hardware notes](docs/hardware.md), [wiring](docs/wiring.md), and
[safety guidance](docs/safety.md) before connecting hardware.

## Wiring summary

| Purpose | Arduino Mega | Connection |
| --- | ---: | --- |
| Swimming Pool Anode | D3 = IN1, D2 = IN2 | L298N |
| Whirlpool Anode | D7 = IN3, D6 = IN4 | L298N |
| LED ring data | D22 | WS2812 DIN |
| Swimming Pool button | D24 | Momentary button to GND |
| Whirlpool button | D26 | Momentary button to GND |
| Mega TX1 | D18 | Voltage divider, then ESP32 GPIO16 / RX2 |
| Mega RX1 | D19 | ESP32 GPIO17 / TX2 |
| Reference | GND | ESP32 GND |

Do not connect the Mega USB port directly to ESP32 USB. Both are USB devices;
use the UART above. Mega D4 and D5 are deliberately not driven because the
L298N ENA and ENB jumpers remain installed.

## Build

Install [PlatformIO Core](https://docs.platformio.org/en/latest/core/installation/index.html).
The ESP32 environment uses Espressif's Arduino Matter library and its required
large application partition.

```sh
pio run -e mega-anode-controller
pio run -e esp32-Matter-gateway
pio test -e native
```

## Flash

Disconnect pool equipment power before changing firmware or low-voltage wiring.
Connect each board to its own USB connection, one at a time.

```sh
pio run -e mega-anode-controller -t upload --upload-port /dev/tty.usbmodemXXXX
pio run -e esp32-Matter-gateway -t upload --upload-port /dev/tty.usbserial-XXXX
```

For a newly flashed Matter board, erase flash before commissioning if it has
old Wi-Fi or Matter fabric data:

```sh
pio run -e esp32-Matter-gateway -t erase
```

## Matter pairing

1. Wire UART2 as documented, then power the Mega and ESP32.
2. Open the ESP32 serial monitor at 115200 baud: `pio device monitor -b 115200`.
3. Copy the printed manual pairing code or open the printed QR-code URL.
4. In a Matter controller, add the device with that code.
5. The two exposed switches are **Swimming Pool Anode** and **Whirlpool
   Anode**. Keep those names when assigning labels in the Matter controller.

See [Matter gateway details](docs/Matter.md), including restart resynchronization.

## Usage

- Press the Swimming Pool button to start its copper anode channel. Press it
  again during startup or runtime to cancel it.
- Press the Whirlpool button to control the Whirlpool Anode channel the same
  way.
- The anode drive output turns on immediately. Its eight LEDs fill orange/red
  without blocking the other channel. The runtime countdown begins after that
  startup animation.
- During runtime, elapsed segments are green; remaining segments are red; the
  active segment blinks. Yellow means cancel feedback and green means finished
  feedback.

## Serial protocol

Messages are ASCII lines terminated with `\n`. The ESP32 sends commands such as
`START_SWIMMING_POOL` and `STOP_WHIRLPOOL`; the Mega emits `READY`, `STATE`,
and `ERROR` events. The full contract and examples are in
[docs/protocol.md](docs/protocol.md).

## Testing

Run the no-hardware unit suite with `pio test -e native`. The complete manual
hardware checklist is in [docs/testing.md](docs/testing.md).

## Repository layout

```text
src/mega-anode-controller/       Arduino Mega hardware integration
src/esp32-Matter-gateway/        ESP32 UART-to-Matter gateway
lib/pool-anode-common/           Testable protocol and anode-runtime logic
test/                            Native Unity tests
docs/                            Hardware, wiring, protocol, Matter, safety, tests
hardware/fritzing/               Fritzing-source guidance and parts location
.github/                         CI, Dependabot, issue and pull-request templates
```

Contributions are welcome; see [CONTRIBUTING.md](CONTRIBUTING.md). This project
uses the [MIT License](LICENSE). It is independent software and is not
affiliated with, endorsed by, or supported by Infinitas. It does not replace
water chemistry testing or professional pool maintenance.

### Dependencies and licensing

This repository does not vendor dependency source. PlatformIO retrieves
Adafruit NeoPixel (LGPL-3.0-or-later), Arduino-ESP32 (LGPL-2.1-or-later), and
ESP-Matter (Apache-2.0) from their upstream projects. Those licenses permit use
alongside MIT-licensed project source, subject to their own notice and
distribution conditions.
