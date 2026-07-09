# HomeKit gateway

The ESP32 runs [HomeSpan](https://github.com/HomeSpan/HomeSpan) and provides a bridge with two switch accessories:

| HomeKit switch | UART request when on | UART request when off | Mega channel |
| --- | --- | --- | --- |
| Pool 30 Minutes | `START_LEFT` | `STOP_LEFT` | Channel A / left |
| Pool 2 Hours | `START_RIGHT` | `STOP_RIGHT` | Channel B / right |

The gateway never drives L298N pins, LED data, buttons, or timer state. A HomeKit toggle is sent over UART but is not accepted optimistically. The switch reflects on only after a Mega `STATE` reports `STARTING` or `RUNNING`; it reflects off for `IDLE` and feedback states.

## Pairing

1. Flash the `esp32-gateway` environment.
2. Watch the ESP32 serial console at 115200 baud during first boot.
3. Complete HomeSpan Wi-Fi provisioning if prompted.
4. Use the pairing code or QR code printed by HomeSpan in Apple Home's **Add Accessory** flow.

HomeSpan manages pairing data on the ESP32. Do not commit pairing data, Wi-Fi credentials, or device-specific setup codes to this repository.

## Resynchronization

On ESP32 boot it immediately sends `STATUS`. On receipt of the Mega's `READY`, it sends `STATUS` again. As a result, restarting the ESP32 does not stop a timer already running on the Mega. If the Mega restarts, its safe boot state turns both loads off; its state event turns both HomeKit switches off after the gateway receives it.
