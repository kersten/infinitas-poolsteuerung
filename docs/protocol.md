# UART protocol

The Arduino Mega is authoritative. The ESP32 sends requests and must wait for a `STATE` event before treating a requested change as accepted. Lines are ASCII and terminated by a newline (`\n`). Carriage returns are ignored.

## Commands: ESP32 to Mega

| Command | Effect |
| --- | --- |
| `START_LEFT` | Start the left timer only when it is idle |
| `STOP_LEFT` | Cancel the left timer when it is starting or running |
| `START_RIGHT` | Start the right timer only when it is idle |
| `STOP_RIGHT` | Cancel the right timer when it is starting or running |
| `TOGGLE_LEFT` | Start idle left timer, otherwise cancel an active left timer |
| `TOGGLE_RIGHT` | Start idle right timer, otherwise cancel an active right timer |
| `STATUS` | Request the current state without changing it |

Commands are idempotent from the gateway's perspective: the Mega responds with a state snapshot even if a start/stop request causes no state change.

## Events: Mega to ESP32

| Event | Meaning |
| --- | --- |
| `READY` | Mega boot completed in the safe, loads-off state |
| `STATE LEFT=<state> RIGHT=<state> LEFT_REMAINING=<ms> RIGHT_REMAINING=<ms>` | Complete authoritative snapshot |
| `ERROR CODE=UNKNOWN_COMMAND MESSAGE=<message>` | Invalid or overlong request was ignored |

States are `IDLE`, `STARTING`, `RUNNING`, `CANCELED_FEEDBACK`, and `FINISHED_FEEDBACK`. Remaining milliseconds equal the configured duration during `STARTING`, decrease only in `RUNNING`, and are zero for every inactive/feedback state.

Examples:

```text
START_LEFT
STATE LEFT=STARTING RIGHT=IDLE LEFT_REMAINING=1800000 RIGHT_REMAINING=0
STATE LEFT=RUNNING RIGHT=IDLE LEFT_REMAINING=1799998 RIGHT_REMAINING=0
STOP_LEFT
STATE LEFT=CANCELED_FEEDBACK RIGHT=IDLE LEFT_REMAINING=0 RIGHT_REMAINING=0
ERROR CODE=UNKNOWN_COMMAND MESSAGE=unsupported_command
```

The Mega emits `STATE` after every channel transition and at least every five seconds while either timer is active. Unknown and malformed input must not change state or crash either board.

## Startup synchronization and restarts

1. On Mega boot, it initializes both loads off, then emits `READY` and a `STATE` snapshot.
2. The ESP32 sends `STATUS` after its own boot, including when the Mega's `READY` was missed.
3. Upon receiving `READY`, the ESP32 sends `STATUS` again to reconcile its Matter endpoint state.
4. Restarting the ESP32 does not alter Mega timers. Restarting the Mega clears both timers and results in an all-off state snapshot.

The parser accepts only complete, exact commands and complete state snapshots; extra or malformed fields are rejected.
