# Wiring guide

> Disconnect mains and low-voltage power before changing wiring. This document describes the controller-side low-voltage connections; it is not a substitute for a qualified mains installation design.

## Mega connections

| Function | Mega pin | Connect to | Notes |
| --- | ---: | --- | --- |
| Channel A ON | D3 | L298N IN1 | HIGH = on |
| Channel A OFF | D2 | L298N IN2 | Keep LOW for off/on operation |
| Channel B ON | D7 | L298N IN3 | HIGH = on |
| Channel B OFF | D6 | L298N IN4 | Keep LOW for off/on operation |
| L298N enable A | D4 | ENA | Leave jumper installed; firmware sets D4 to input only |
| L298N enable B | D5 | ENB | Leave jumper installed; firmware sets D5 to input only |
| Ring data | D22 | WS2812 DIN | Add data resistor near ring input |
| Left button | D24 | Button → GND | `INPUT_PULLUP`, active-low |
| Right button | D26 | Button → GND | `INPUT_PULLUP`, active-low |

## UART between boards

| From | To | Required conditioning |
| --- | --- | --- |
| Mega D18 / TX1 | ESP32 GPIO16 / RX2 | Mega D18 → 1 kΩ → GPIO16; GPIO16 → 2 kΩ → GND |
| ESP32 GPIO17 / TX2 | Mega D19 / RX1 | Direct connection is acceptable |
| Mega GND | ESP32 GND | Mandatory common reference |

The divider is mandatory because Mega output is 5 V. Never put a 5 V Mega TX signal directly into an ESP32 GPIO. Do not join Mega USB and ESP32 USB with a USB cable.

## External control box

| Device | Connection |
| --- | --- |
| Left button | One terminal to D24, other terminal to GND |
| Right button | One terminal to D26, other terminal to GND |
| WS2812 DIN | Mega D22 through a 330–470 Ω series resistor |
| WS2812 5 V | Regulated 5 V supply suitable for 16 pixels |
| WS2812 GND | Common low-voltage ground |

Place a roughly 1000 µF electrolytic capacitor across 5 V and GND close to the LED ring, observing polarity. Place the 330–470 Ω data resistor close to the ring's DIN pin. The one-metre cable should have proper strain relief and a ground conductor alongside the data wire; if interference occurs, shorten the run or use a suitable level-shifting/buffer solution designed for the installation.

## Pre-power checklist

- Confirm L298N ENA/ENB jumpers are installed.
- Confirm D4 and D5 are not used as control wires or firmware outputs.
- Verify the Mega-to-ESP32 divider values and common ground.
- Verify ring polarity, capacitor polarity, and data direction.
- Test with pool loads safely isolated before enabling any controlled equipment.
