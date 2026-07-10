# Hardware

pool-anode-controller separates safety-critical copper-anode control from
network integration.

| Component | Responsibility |
| --- | --- |
| Arduino Mega 2560 | Authoritative anode state, buttons, LED ring, runtime, and L298N input signals |
| ESP32 | Matter gateway; UART commands and confirmed-state mirroring only |
| Existing Infinitas L298N | Drives both copper anode output pairs |
| WS2812 / NeoPixel ring | 16 status LEDs: 8 Swimming Pool Anode, 8 Whirlpool Anode |
| External control box | Two simple momentary buttons and the LED ring, about 1 m from the main box |
| Buck converter | Provides a correctly rated, stable supply for low-voltage electronics |
| UART voltage divider | Reduces Mega 5 V TX1 to an ESP32-safe 3.3 V RX2 signal |

## Existing L298N mapping

| Copper anode channel | Arduino Mega | L298N | Active polarity |
| --- | ---: | --- | --- |
| Swimming Pool Anode | D3 | IN1 | HIGH |
| Swimming Pool Anode | D2 | IN2 | LOW |
| Whirlpool Anode | D7 | IN3 | HIGH |
| Whirlpool Anode | D6 | IN4 | LOW |

The Mega configures each pair as outputs. Its safe startup condition drives all
four input signals LOW, so both copper anode channels are off. The ON-polarity
constants are at the top of `src/mega-anode-controller/main.cpp`; validate
polarity with the installed ionization system before normal operation.

## L298N enable jumpers

Keep the L298N ENA and ENB jumpers installed. They hold the bridge enables
active. Therefore Mega D4 and D5 are configured as inputs and are otherwise
unused. Firmware never configures either pin as an output and never writes or
uses PWM on either pin. This avoids electrical contention with the installed
jumper arrangement.

## Runtime ownership

The Mega starts the appropriate copper anode drive output immediately, runs the
startup animation, then starts the configured runtime countdown. It also owns
cancel and finished feedback. The ESP32 has no direct connection to the L298N
inputs or LED ring, and no runtime state of its own.

This project controls runtime only. It does not measure copper concentration,
pH, alkalinity, flow, or any other water-chemistry parameter. Validate settings
using appropriate pool-test equipment and professional guidance.
