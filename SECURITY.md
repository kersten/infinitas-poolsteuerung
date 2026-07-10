# Security policy

## Reporting a vulnerability

Do not publicly disclose a suspected safety-critical vulnerability before
maintainers have had a reasonable opportunity to respond. Use GitHub private
security advisories when enabled, or contact the repository owner privately
with a subject such as `Security report: pool-anode-controller`.

Include the affected revision, a safe proof of concept, expected impact, and
suggested mitigation. Do not energize pool equipment merely to demonstrate a
report.

## Scope

Security and safety scope includes:

- firmware defects that can unexpectedly activate a copper anode output;
- Matter or UART command handling defects that bypass Arduino state authority;
- unsafe wiring documentation, including UART level-shifting guidance;
- credentials or Matter pairing information accidentally committed to source.

Maintainers will acknowledge reports, assess severity, coordinate a correction
where practical, and credit reporters who wish to be named.
