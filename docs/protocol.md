# UART protocol

The Arduino Mega owns the true copper-anode state. The ESP32 sends requests;
it does not assume they succeeded until it receives a `STATE` event. Every
message is one ASCII line ending in `\n`. Receivers ignore `\r`.

## ESP32-to-Mega commands

| Command | Behavior |
| --- | --- |
| `START_SWIMMING_POOL` | Start the Swimming Pool Anode only when idle |
| `STOP_SWIMMING_POOL` | Cancel the Swimming Pool Anode when starting or running |
| `START_WHIRLPOOL` | Start the Whirlpool Anode only when idle |
| `STOP_WHIRLPOOL` | Cancel the Whirlpool Anode when starting or running |
| `TOGGLE_SWIMMING_POOL` | Start idle Swimming Pool Anode or cancel active channel |
| `TOGGLE_WHIRLPOOL` | Start idle Whirlpool Anode or cancel active channel |
| `STATUS` | Request the current authoritative snapshot |

Start and stop no-ops still cause the Mega to send its state so the gateway can
reconcile.

## Mega-to-ESP32 events

| Event | Meaning |
| --- | --- |
| `READY` | Mega booted with both copper anode outputs off |
| `STATE SWIMMING_POOL=<state> WHIRLPOOL=<state> SWIMMING_POOL_REMAINING=<ms> WHIRLPOOL_REMAINING=<ms>` | Complete authoritative snapshot |
| `ERROR CODE=UNKNOWN_COMMAND MESSAGE=<message>` | Command was not recognized or line was too long |

States are `IDLE`, `STARTING`, `RUNNING`, `CANCELED_FEEDBACK`, and
`FINISHED_FEEDBACK`. The Matter gateway maps `STARTING` and `RUNNING` to switch
on; all other states map to switch off.

Examples:

```text
READY
STATE SWIMMING_POOL=IDLE WHIRLPOOL=IDLE SWIMMING_POOL_REMAINING=0 WHIRLPOOL_REMAINING=0
START_SWIMMING_POOL
STATE SWIMMING_POOL=STARTING WHIRLPOOL=IDLE SWIMMING_POOL_REMAINING=1800000 WHIRLPOOL_REMAINING=0
STATE SWIMMING_POOL=RUNNING WHIRLPOOL=RUNNING SWIMMING_POOL_REMAINING=1234567 WHIRLPOOL_REMAINING=7654321
ERROR CODE=UNKNOWN_COMMAND MESSAGE=unsupported_command
```

## Synchronization and errors

The Mega emits `READY`, followed by a state snapshot, during startup. It emits
a new snapshot after anode-channel state changes and at least every five
seconds while either channel is active. The ESP32 asks for `STATUS` after its
own boot and again whenever it receives `READY`.

An unknown or malformed command must not change either anode channel. The Mega
returns `ERROR`; the ESP32 logs unknown or malformed inbound lines and leaves
its confirmed Matter state untouched. A Mega restart returns to a safe,
both-off condition and the next snapshot corrects Matter state.
