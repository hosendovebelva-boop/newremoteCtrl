# RemoteAssist

A consent-based LAN screen-sharing sample built with MFC and Winsock.

## What this repo contains

This repository now ships three Visual Studio projects inside `RemoteCtrl/RemoteAssist.sln`:

- `AssistHost`: a visible host app that shows local IPs, generates a 6-digit session code, asks for local consent, and displays a session banner while sharing is active.
- `AssistViewer`: a visible viewer app that connects to the host, sends a `Hello` handshake with the session code plus a helper display name, waits for host approval, and displays read-only screen updates.
- `PacketTests`: a small console test project that exercises the shared packet parser and `Hello` payload validation.

## Safety boundaries

This codebase is intentionally scoped to a narrow, visible remote-assistance workflow.

- No persistence
- No elevation
- No remote command execution
- No file browsing, upload, download, or deletion
- No mouse or keyboard injection
- No lock-screen behavior
- One viewer at a time
- Plaintext LAN transport only

## Session flow

1. Launch `AssistHost` manually.
2. Read the 6-digit session code from the host window.
3. Enter the host IP, port, session code, and helper name into `AssistViewer`.
4. The host receives the request and shows an allow/deny dialog with a 30-second timeout.
5. If allowed, the viewer requests a PNG frame every 500 ms.
6. Ending the session closes the TCP connection; there is no explicit remote-control command channel.

## Project layout

- `RemoteCtrl/RemoteCtrl/`: host app, consent dialog, banner window, server loop, screen capture
- `RemoteCtrl/RemoteClient/`: viewer app, persistent client socket, watch dialog
- `RemoteCtrl/ScreenShareProtocol.h`: shared protocol constants, command ids, and `Hello` helpers
- `RemoteCtrl/SharedPacket.h`: shared packet serialization and parsing
- `RemoteCtrl/PacketTests/`: parser and handshake regression tests
- `RemoteCtrl/THREAT_MODEL.md`: current risks and assumptions

## Build

Open `RemoteCtrl/RemoteAssist.sln` in Visual Studio 2022 and build:

- `AssistHost`
- `AssistViewer`
- `PacketTests`

Recommended configurations:

- `Debug|x64`
- `Release|x64`

## Current limitations

- LAN only
- Plaintext transport
- No TLS or identity binding beyond the session code plus visible consent
- No multi-viewer support
- No reconnect or resume
- Read-only PNG screen streaming only

## Legacy material

`RemoteCtrl/RemoteControlSystemDesign.mdj` is retained only as legacy reference material from the earlier remote-control study code. It is not part of the active product path.
