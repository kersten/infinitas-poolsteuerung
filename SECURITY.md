# Security policy

## Reporting a vulnerability

Please do not publicly disclose a suspected safety-critical vulnerability before maintainers have had a reasonable opportunity to respond. Use GitHub's private security advisory reporting for this repository when available; otherwise contact the maintainers privately through the repository owner profile with a clear subject such as `Security report: pool timer`.

Include the affected revision, a safe proof of concept, impact, and any suggested mitigation. Do not perform tests on energized mains equipment solely to demonstrate a report.

## Scope

Security and safety scope includes:

- firmware defects that can unexpectedly activate or fail to deactivate loads;
- HomeKit or UART command handling that bypasses Arduino state authority;
- unsafe or misleading wiring documentation, including level-shifting guidance;
- credentials or pairing information accidentally committed to the repository.

Maintainers will acknowledge reports, assess severity, coordinate a fix where practical, and credit reporters who wish to be named.
