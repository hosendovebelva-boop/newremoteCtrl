# ScreenShareHost / ScreenShareViewer

A consent-based LAN screen sharing sample built with MFC and Winsock.

## What this repo contains

This repository now ships two Windows applications inside `RemoteCtrl/RemoteCtrl.sln`:

- `ScreenShareHost`: a visible host app that shows local IPs, generates a 6-digit session code, asks for consent, and displays a top banner while sharing is active.
- `ScreenShareViewer`: a visible viewer app that connects to the host, submits the session code, waits for host approval, and displays read-only screen updates.
- `PacketTests`: a small console test project that exercises the shared packet parser.

## Safety boundaries

This codebase is intentionally scoped to a narrow, visible screen-sharing workflow.

- No persistence
- No elevation
- No remote command execution
- No file browsing, upload, download, or deletion
- No mouse or keyboard injection
- No lock-screen behavior
- One viewer at a time

## Session flow

1. Launch `ScreenShareHost` manually.
2. Read the 6-digit session code from the host window.
3. Enter the host IP, port, and session code into `ScreenShareViewer`.
4. The host receives the request and shows an allow/deny dialog.
5. If allowed, the viewer requests a frame every 500 ms.
6. Either side can end the session explicitly.

## Project layout

- `RemoteCtrl/RemoteCtrl/`: host app, consent dialog, banner window, server loop, screen capture
- `RemoteCtrl/RemoteClient/`: viewer app, persistent client socket, watch dialog
- `RemoteCtrl/ScreenShareProtocol.h`: shared protocol constants and command/status values
- `RemoteCtrl/SharedPacket.h`: shared packet serialization and parsing
- `RemoteCtrl/PacketTests/`: parser regression tests
- `RemoteCtrl/THREAT_MODEL.md`: current risks and assumptions

## Build

Open `RemoteCtrl/RemoteCtrl.sln` in Visual Studio 2022 and build:

- `ScreenShareHost`
- `ScreenShareViewer`
- `PacketTests`

Recommended configurations:

- `Debug|x64`
- `Release|x64`

## Current limitations

- LAN only
- Plaintext transport
- No TLS or identity binding beyond the session code
- No multi-viewer support
- No reconnect or resume

## Legacy material

`RemoteCtrl/RemoteControlSystemDesign.mdj` is retained only as legacy reference material from the earlier remote-control study code. It is not part of the active product path.
