# Testing

## Native unit tests

Shared anode logic has no Arduino hardware calls and runs on the host:

```sh
pio test -e native
```

The suite covers protocol parsing and serialization, anode-channel transitions,
remaining runtime, `millis()` overflow-safe arithmetic, button debounce, and
configurable LED mapping.

## Firmware builds

```sh
pio run -e mega-anode-controller
pio run -e esp32-Matter-gateway
```

## Manual hardware checklist

Perform these only with a safe, de-energized installation process and suitable
test equipment. Do not treat a successful firmware build as water-chemistry
validation.

- [ ] Arduino boot condition: both copper anode outputs are off and D4/D5 are
  inputs.
- [ ] ESP32 Matter pairing completes and shows Swimming Pool Anode and
  Whirlpool Anode.
- [ ] UART communication produces `READY`, `STATUS`, and `STATE` behavior.
- [ ] Swimming Pool Anode starts from its local button and its runtime matches
  the configured 30 minutes after startup animation.
- [ ] Whirlpool Anode starts from its local button and its runtime matches the
  configured 2 hours after startup animation.
- [ ] Both anode channels operate simultaneously without changing each other.
- [ ] A second local press cancels the corresponding anode output immediately
  and yellow feedback appears.
- [ ] Finished runtime disables the corresponding anode output and green
  feedback appears.
- [ ] Each LED half fills non-blockingly on startup, then shows runtime
  segments.
- [ ] Restarting ESP32 does not interrupt an active Mega anode runtime; Matter
  state resynchronizes.
- [ ] Restarting Mega leaves both anode outputs off; Matter updates from the
  new state.
- [ ] Independently validate copper, pH, alkalinity, and other required water
  parameters with appropriate pool-test equipment.
