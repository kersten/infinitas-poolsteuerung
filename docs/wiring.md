# Wiring

Disconnect all power before wiring. This document covers low-voltage controller
connections; mains work belongs to qualified persons under local regulations.

## L298N copper anode connections

| Copper anode channel | Mega pin | L298N input | Mega state when enabled |
| --- | ---: | --- | --- |
| Swimming Pool Anode | D3 | IN1 | HIGH |
| Swimming Pool Anode | D2 | IN2 | LOW |
| Whirlpool Anode | D7 | IN3 | HIGH |
| Whirlpool Anode | D6 | IN4 | LOW |

Leave L298N ENA and ENB jumpers installed. D4 and D5 must not be wired as
firmware-controlled enables and are not driven by the firmware.

## UART: Arduino Mega Serial1 to ESP32 UART2

| Signal | From | To | Required treatment |
| --- | --- | --- | --- |
| Mega TX1 | D18 | ESP32 RX2 GPIO16 | Pass through a 5 V-to-3.3 V level shifter or voltage divider |
| ESP32 TX2 | GPIO17 | Mega RX1 D19 | Direct 3.3 V UART signal is normally accepted by the Mega |
| Ground | Mega GND | ESP32 GND | Common reference is mandatory |

Do not connect the boards' USB ports to each other. Use their USB ports only
individually for flashing or serial diagnostics.

For a voltage divider on the Mega-to-ESP32 line, use a resistor from D18 to the
GPIO16 node and another from that node to ground, selected so a 5 V input is at
or below ESP32 3.3 V logic level. A common starting point is 1 kΩ high side and
2 kΩ low side; verify values, tolerance, and installation suitability.

## External control box

| Item | Mega connection | Other connection | Notes |
| --- | ---: | --- | --- |
| Swimming Pool button | D24 | GND | `INPUT_PULLUP`, active-low |
| Whirlpool button | D26 | GND | `INPUT_PULLUP`, active-low |
| WS2812 ring DIN | D22 | Ring data input | Put series data resistor close to the first LED |
| WS2812 ring 5 V | Rated 5 V supply | Ring VCC | Size supply and cable appropriately |
| WS2812 ring GND | Common GND | Ring GND | Required with data signal |

The buttons have no LEDs: each normally-open switch simply joins its Mega pin
to ground when pressed. The firmware debounces them in software.

Place a roughly 330–470 Ω resistor in the LED data wire near the first LED and
a suitably rated electrolytic capacitor (for example, 500–1000 µF) across the
ring 5 V and GND close to the ring. Follow the ring manufacturer's voltage,
current, and polarity specifications, especially for the approximately 1 m
cable.

## Installation checks

1. With all power removed, inspect polarity, insulation, strain relief, and
   common ground connections.
2. Power low-voltage electronics safely and verify both copper anode inputs
   remain LOW at Arduino startup.
3. Verify UART `READY` and `STATE` messages before Matter commissioning.
4. Test the Swimming Pool Anode and Whirlpool Anode separately, then together,
   before connecting the system to normal pool operation.
