# Testing

## Native tests

The `native` PlatformIO environment runs pure C++ tests without hardware:

```sh
pio test -e native
```

Coverage includes command/state parsing and serialization, timer state transitions and overflow-safe time arithmetic, active-low debouncing, and LED physical mapping/orientation.

## Firmware builds

```sh
pio run -e mega-controller
pio run -e esp32-gateway
```

The GitHub Actions workflow runs these builds and the native tests for pull requests and pushes to `main`.

## Manual hardware test checklist

Perform hardware tests with a safe load/test setup before connecting live pool equipment.

- [ ] Arduino boot: both L298N channels are off; D4 and D5 are inputs and ENA/ENB jumpers remain installed.
- [ ] ESP32 can provision and pair with Apple Home through HomeSpan.
- [ ] UART: Mega sends `READY`/`STATE`; ESP32 requests `STATUS` and receives snapshots.
- [ ] Left timer: local left button starts Channel A and its 8-pixel half.
- [ ] Right timer: local right button starts Channel B and its 8-pixel half.
- [ ] Simultaneous timers: both channels run without interfering.
- [ ] Cancel behavior: pressing an active channel's button stops its load immediately and flashes yellow, then turns its half off.
- [ ] Finish behavior: expiry stops its load and flashes green, then turns its half off.
- [ ] Startup animation: each half fills non-blockingly, pulses briefly, and only then starts its countdown.
- [ ] HomeKit: each switch sends the correct UART start/stop request and follows confirmed Mega state.
- [ ] ESP32 restart: an active Mega timer continues and switch state returns after `STATUS`.
- [ ] Arduino restart: both loads are off and HomeKit updates to off after the new state snapshot.

Record the tested firmware revision, hardware revision, power conditions, and any deviations in a pull request or issue.
