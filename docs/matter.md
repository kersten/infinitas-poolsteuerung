# Matter gateway

The ESP32 runs [ESP-Matter](https://github.com/espressif/esp-matter) and exposes two Matter Pump endpoints. Matter controllers can display them as controllable on/off accessories; name them **Pool 30 Minutes** and **Pool 2 Hours** in the controller after commissioning.

| Matter endpoint | UART request when on | UART request when off | Mega channel |
| --- | --- | --- | --- |
| Pool 30 Minutes | `START_LEFT` | `STOP_LEFT` | Channel A / left |
| Pool 2 Hours | `START_RIGHT` | `STOP_RIGHT` | Channel B / right |

The gateway never drives L298N pins, LED data, buttons, or timer state. It sends a UART request for a Matter action, rejects the speculative Matter attribute write, then publishes the state only after a Mega `STATE` event. `STARTING` and `RUNNING` report on; `IDLE` and feedback states report off.

## SDK setup and build

This project pins the gateway integration to **ESP-Matter v1.4.2** and **ESP-IDF v5.4.1**, the ESP-Matter release's recommended ESP-IDF version. Follow Espressif's [ESP-Matter setup guide](https://docs.espressif.com/projects/esp-matter/en/latest/esp32/developing.html#getting-the-repositories) to install those SDKs and initialize their submodules.

In a terminal where ESP-IDF has been exported, set and export the checked-out ESP-Matter path:

```sh
export ESP_MATTER_PATH=/absolute/path/to/esp-matter
. "$ESP_MATTER_PATH/export.sh"

idf.py -C esp32-gateway set-target esp32
idf.py -C esp32-gateway build
idf.py -C esp32-gateway -p /dev/tty.usbserial-XXXX flash monitor
```

The project includes the 4 MiB ESP32-WROOM partition layout used by the ESP-Matter light example, with two OTA slots. Build output and generated `sdkconfig` files are local artifacts and must not be committed.

## Commissioning and reset

1. Flash the gateway and open the ESP-IDF monitor at 115200 baud.
2. ESP-Matter prints a QR code and manual setup code on first boot.
3. In Apple Home, select **Add Accessory**, then scan or enter the Matter setup code. Matter carries the Wi-Fi credentials during commissioning; there is no separate Wi-Fi provisioning flow.
4. Rename the two accessories in the controller to match their timer durations.

To clear Matter fabrics, pairing data, and Wi-Fi credentials during development, erase the ESP32 flash and reflash:

```sh
idf.py -C esp32-gateway erase-flash
```

The stock ESP-Matter credentials and setup payload are for development only. A distributed product needs its own Matter vendor/product identifiers and CSA-compliant device-attestation credentials.

## Resynchronization

On ESP32 boot the gateway immediately sends `STATUS`. On receipt of the Mega's `READY`, it sends `STATUS` again. Restarting the ESP32 therefore does not stop a timer already running on the Mega. If the Mega restarts, its safe boot state turns both loads off; the subsequent `STATE` snapshot updates both Matter endpoints to off.
