# RemoteAssist Threat Model

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
- Per-stream consent state for future screen, microphone, and camera streams
- Persistent host-side banner while screen sharing is active
- Append-only local session log
- 60-minute default session limit with local-only extension

## Remaining risks

### Plaintext transport

Traffic is not encrypted. Anyone on the same network path can observe the packet stream and captured PNG frames.

### Session code is not identity

The 6-digit code plus helper display name is a visible session gate, not a cryptographic identity check. A local attacker could guess the code if they can repeatedly connect.

### Limited brute-force resistance

Each TCP connection gets three wrong attempts before the host closes it, but there is no cross-connection IP ban or account lockout.

### No replay or tamper protection

The packet checksum only protects parser integrity. It is not authentication and does not prevent malicious packet injection by an active network attacker.

### Single-monitor assumptions

The current capture path uses the primary monitor only.

### Local log integrity

The session log is append-only by application behavior, but it is a local user-writable text file and is not tamper-proof.

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
- Share the session code only with the intended helper.
- Confirm the helper name in the consent dialog before approving.
- End the session when you are done instead of leaving the host running unnecessarily.
