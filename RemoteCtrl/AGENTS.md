# AGENTS.md

## Repository summary

`RemoteCtrl/RemoteAssist.sln` now builds three projects:

- `AssistHost`
- `AssistViewer`
- `PacketTests`

This repository is a consent-based remote-assistance sample. It is not a remote-administration or remote-control tool.

## Source layout

- `RemoteCtrl/`: host-side MFC app
- `RemoteClient/`: viewer-side MFC app
- `ScreenShareProtocol.h`: shared command/status constants plus `Hello` payload helpers
- `SharedPacket.h`: shared packet serializer/parser
- `PacketTests/`: packet parser and handshake tests

## Active behaviors

- Host is always visible
- Viewer must send a 6-digit session code plus helper display name
- Host must explicitly allow the request
- The consent dialog auto-denies after 30 seconds
- Viewer can only request screenshots
- Viewer requests frames every 500 ms
- Ending a session closes the TCP connection

## Explicitly removed capabilities

- Persistence
- Elevation
- Remote execution
- File system control
- Mouse control
- Lock-screen behavior

## Build notes

Open `RemoteCtrl/RemoteAssist.sln` in Visual Studio 2022.

Primary configs:

- `Debug|x64`
- `Release|x64`

## Review notes

When reviewing changes, prioritize:

- shared packet correctness
- `Hello` payload validation and consent flow
- host visibility guarantees
- viewer frame pacing and read-only behavior
- regressions that accidentally reintroduce remote-control capability
