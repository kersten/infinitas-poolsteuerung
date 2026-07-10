# Matter gateway

The ESP32 is a UART-to-Matter gateway, not a copper-anode controller. Its two
Matter switches are **Swimming Pool Anode** and **Whirlpool Anode**. The Mega
is the source of truth for both switch values.

## Switch mapping

| Matter switch request | UART command | Confirmed switch state |
| --- | --- | --- |
| Swimming Pool Anode on | `START_SWIMMING_POOL` | On after Mega reports `SWIMMING_POOL=STARTING` or `RUNNING` |
| Swimming Pool Anode off | `STOP_SWIMMING_POOL` | Off after Mega reports `SWIMMING_POOL=IDLE` or feedback state |
| Whirlpool Anode on | `START_WHIRLPOOL` | On after Mega reports `WHIRLPOOL=STARTING` or `RUNNING` |
| Whirlpool Anode off | `STOP_WHIRLPOOL` | Off after Mega reports `WHIRLPOOL=IDLE` or feedback state |

The gateway does not make an anode output, LED, or runtime decision. It sends a
command and waits for a Mega `STATE` line. This can make a controller UI wait
briefly while the UART acknowledgement arrives, which is intentional.

## Pairing

1. Build and flash `esp32-Matter-gateway` with PlatformIO.
2. Open the ESP32 serial monitor at 115200 baud.
3. If uncommissioned, the firmware periodically prints a manual pairing code
   and QR-code URL.
4. Add the device in a Matter controller using the printed code.
5. Keep or assign the two switch labels as Swimming Pool Anode and Whirlpool
   Anode in the controller interface.

Matter requires the large application partition configured in `platformio.ini`.
For a fresh board or commissioning recovery, erase the ESP32 flash, reflash,
and pair again. See Espressif's Matter documentation for controller-specific
network setup.

## Restart recovery

On ESP32 boot, the gateway sends `STATUS`, so it learns a still-active Mega
anode runtime without restarting it. On Mega boot, both copper anode outputs
are off before UART starts; it emits `READY` and a state snapshot. The gateway
asks for `STATUS` after `READY` and updates both Matter switches accordingly.
