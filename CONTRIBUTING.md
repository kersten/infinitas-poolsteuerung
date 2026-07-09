# Contributing

Thanks for contributing. This controller can affect pool equipment, so please treat safety, reviewability, and test coverage as first-class requirements.

## Setup and checks

Install [PlatformIO Core](https://docs.platformio.org/en/latest/core/installation/index.html), ESP-IDF v5.4.1, and ESP-Matter v1.4.2; then fork the repository and create a focused branch. Run these checks before opening a pull request:

```sh
pio run -e mega-controller
pio test -e native
idf.py -C esp32-gateway build
```

Use concise, imperative commits such as `Add UART status timeout` or `Fix right timer feedback blink`.

## Issues and pull requests

- Search existing issues before creating a new one. Include versions, safe reproduction steps, and relevant serial output.
- Keep pull requests small and explain the behavior change and validation performed.
- Never claim hardware behavior that you have not tested. State when testing is limited to native tests or compilation.
- Hardware-related changes must document their wiring impact in `docs/wiring.md` and `docs/hardware.md` as appropriate.
- Protocol changes must update `docs/protocol.md` and the relevant native tests in the same pull request.
- Mega changes must preserve the invariant that D4 and D5 are never driven as outputs while the L298N enable jumpers are installed.

By participating, you agree to follow the [Code of Conduct](CODE_OF_CONDUCT.md).
