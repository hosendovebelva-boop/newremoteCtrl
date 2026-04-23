# AGENTS.md

## Repository summary

`RemoteCtrl/RemoteCtrl.sln` now builds three projects:

- `ScreenShareHost`
- `ScreenShareViewer`
- `PacketTests`

This repository is a consent-based screen sharing sample. It is no longer a remote-administration or remote-control tool.

## Source layout

- `RemoteCtrl/`: host-side MFC app
- `RemoteClient/`: viewer-side MFC app
- `ScreenShareProtocol.h`: shared command/status constants
- `SharedPacket.h`: shared packet serializer/parser
- `PacketTests/`: packet parser test project

## Active behaviors

- Host is always visible
- Viewer must submit a 6-digit session code
- Host must explicitly allow the request
- Viewer can only request screenshots
- Viewer requests frames every 500 ms
- Either side can end the session

## Explicitly removed capabilities

- Persistence
- Elevation
- Remote execution
- File system control
- Mouse control
- Lock-screen behavior

## Build notes

Open `RemoteCtrl/RemoteCtrl.sln` in Visual Studio 2022.

Primary configs:

- `Debug|x64`
- `Release|x64`

## Review notes

When reviewing changes, prioritize:

- shared packet correctness
- session code and consent flow
- host visibility guarantees
- viewer frame pacing and read-only behavior
- regressions that accidentally reintroduce remote-control capability
