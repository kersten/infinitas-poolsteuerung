# Hardware overview

## Safety boundary

The Arduino Mega controls existing pool loads through an L298N driver and may be installed alongside mains-voltage equipment. The ESP32 and external controls are low-voltage only; they do not make an installation mains-safe. Isolate power before opening the enclosure and have qualified persons perform mains work under local rules.

## Components

| Component | Role |
| --- | --- |
| Arduino Mega 2560 | Source of truth for timer state, buttons, load outputs, and LEDs |
| ESP32 DevKit | HomeSpan/HomeKit gateway over UART only |
| Existing L298N | Drives the two existing Infinitas load channels |
| WS2812 ring | Sixteen visual status pixels: eight per timer |
| Two momentary buttons | Local start/cancel controls, wired active-low |
| Buck converter | Provides a suitable regulated low-voltage supply; size and isolate appropriately |
| 1 kΩ + 2 kΩ resistors | UART voltage divider from Mega TX1 to ESP32 RX2 |

## Existing Infinitas L298N mapping

| Arduino pin | L298N pin | Meaning |
| ---: | --- | --- |
| D3 | IN1 | Channel A forward/on signal |
| D2 | IN2 | Channel A off signal |
| D7 | IN3 | Channel B forward/on signal |
| D6 | IN4 | Channel B off signal |
| D4 | ENA | **Not driven** |
| D5 | ENB | **Not driven** |

Channel A is on when IN1 is HIGH and IN2 is LOW; it is off when both are LOW. Channel B is on when IN3 is HIGH and IN4 is LOW; it is off when both are LOW.

The L298N ENA and ENB jumpers must remain installed. They hold the enable inputs active, so this firmware configures Mega D4 and D5 as `INPUT` and never writes or PWM-drives them. Removing the jumpers changes the electrical design and is outside this project's supported configuration.

## External control box

The approximately one-metre external box contains two simple buttons and the 16-pixel ring. The Mega uses internal pull-ups, so each button connects from its signal pin to GND. The ring uses Mega D22 as data. Keep the low-voltage control cable separate from mains wiring, provide strain relief, and use a common low-voltage ground.

## UART level shifting

The Mega's Serial1 logic is 5 V and ESP32 GPIO is 3.3 V. The Mega TX1/D18 signal **must** be divided before reaching ESP32 GPIO16/RX2. A 1 kΩ series resistor from D18 to GPIO16 and a 2 kΩ resistor from GPIO16 to GND produces approximately 3.3 V. ESP32 GPIO17/TX2 can connect directly to Mega D19/RX1, and the boards must share ground.

Do not connect the two USB ports together. Both boards are USB devices; use their separate USB connections only for power/programming/debugging and use UART for inter-board communication.
