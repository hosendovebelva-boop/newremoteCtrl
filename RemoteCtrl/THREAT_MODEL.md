# Threat Model

## Intended environment

- Same-LAN usage
- One host
- One viewer
- Manual launch on both sides
- Human-in-the-loop consent on the host

## Deliberate safety properties

- No persistence
- No privilege escalation
- No hidden background execution
- No remote execution
- No file transfer or deletion
- No input injection
- No lock-screen behavior

## Remaining risks

### Plaintext transport

Traffic is not encrypted. Anyone on the same network path can observe the packet stream and captured PNG frames.

### Session code is not identity

The 6-digit code is a per-session gate, not a cryptographic identity check. A local attacker could guess it if they can repeatedly connect.

### Limited brute-force resistance

Each TCP connection gets three wrong attempts before the host closes it, but there is no cross-connection IP ban or account lockout.

### No replay or tamper protection

The packet checksum only protects parser integrity. It is not authentication and does not prevent malicious packet injection by an active network attacker.

### Single-monitor assumptions

The current capture path uses the primary monitor only.

## Non-goals

- Internet exposure
- Multi-user identity management
- TLS
- Persistent pairing
- Reconnect/resume
- Multi-viewer fan-out

## Operational guidance

- Keep usage to trusted LAN environments.
- Do not expose the host port to the public internet.
- Read the session code aloud only to the intended viewer.
- End the session when you are done instead of leaving the host running unnecessarily.
