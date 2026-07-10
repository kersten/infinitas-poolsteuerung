# Safety

pool-anode-controller is a DIY project intended for people who understand the
risks around pool equipment. It is not a substitute for qualified electrical
work, water testing, or professional pool maintenance.

## Electrical safety

- Disconnect power before opening a box, changing wiring, or attaching test
  equipment.
- Mains wiring and enclosure changes must be completed by qualified persons
  under local regulations.
- Use appropriately rated enclosures, conductors, fuses, power conversion,
  strain relief, and grounding.
- Never connect Arduino Mega USB directly to ESP32 USB. Use the documented
  UART, common ground, and Mega-TX-to-ESP32-RX level shifting.
- Keep L298N ENA and ENB jumpers installed only when D4 and D5 are not driven;
  this firmware intentionally keeps them as inputs.

## Pool-equipment and water safety

This firmware does not measure copper concentration, pH, alkalinity,
sanitizer, temperature, flow, or any other water parameter. It only sets a
time window for two copper anode channels. Runtime values must be validated
manually with real water tests and adjusted with qualified advice.

Incorrect copper concentration can cause staining, corrosion, water-quality
problems, and equipment damage. Evaluate any interaction with the existing
ionization equipment and pool chemistry before normal use. You use this
project entirely at your own risk.
