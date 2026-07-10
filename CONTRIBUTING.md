# Contributing

Thank you for contributing to pool-anode-controller. Electrical and water
safety are part of every change, not an afterthought.

## Setup and verification

Install [PlatformIO Core](https://docs.platformio.org/en/latest/core/installation/index.html),
fork the repository, and create a focused branch. Before opening a pull request,
run:

```sh
pio run -e mega-anode-controller
pio run -e esp32-Matter-gateway
pio test -e native
```

Use concise, imperative commits, for example `Add UART state timeout` or
`Fix Whirlpool Anode feedback blink`.

## Issues and pull requests

- Search open issues first and include firmware versions, safe reproduction
  steps, and relevant serial output.
- Keep each pull request focused and state exactly which checks and hardware
  tests were performed.
- Hardware-related changes must document their wiring impact in
  `docs/wiring.md` and `docs/hardware.md`.
- Protocol changes must update `docs/protocol.md` and corresponding native tests
  in the same pull request.
- Preserve the safety invariant that Mega D4 and D5 are never output pins while
  the L298N enable jumpers are installed.
- Use Swimming Pool Anode, Whirlpool Anode, `swimmingPool`, and `whirlpool`
  terminology consistently. Do not reintroduce positional or generic channel
  names.

By participating, you agree to the [Code of Conduct](CODE_OF_CONDUCT.md).
