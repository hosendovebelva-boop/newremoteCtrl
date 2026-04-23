# CLAUDE.md

This repository now contains a consent-based screen-sharing sample, not a remote-administration tool.

## Solution

- Solution: `RemoteCtrl/RemoteAssist.sln`
- Projects:
  - `AssistHost` -> `RemoteCtrl/RemoteCtrl/AssistHost.vcxproj`
  - `AssistViewer` -> `RemoteCtrl/RemoteClient/AssistViewer.vcxproj`
  - `PacketTests` -> `RemoteCtrl/PacketTests/PacketTests.vcxproj`

## Core shared files

- `RemoteCtrl/ScreenShareProtocol.h`
- `RemoteCtrl/SharedPacket.h`

These two files define the wire contract used by both applications.

## Product behavior

- `AssistHost` is always visible.
- The host generates a 6-digit session code per session.
- The viewer must send that code plus a helper name in the `Hello` handshake before the host user is asked for consent.
- The host must explicitly allow the request.
- The viewer can only request screenshots.
- Screenshot requests are paced at 500 ms intervals.
- Ending the session closes the TCP connection.

## Out of scope by design

- No persistence
- No elevation
- No remote execution
- No file transfer
- No file deletion
- No mouse or keyboard control
- No lock-screen functionality

## Important host files

- `RemoteCtrl/RemoteCtrl/RemoteCtrl.cpp`: MFC app entry
- `RemoteCtrl/RemoteCtrl/HostMainDlg.*`: visible host UI and tray handling
- `RemoteCtrl/RemoteCtrl/ServerSocket.*`: persistent single-viewer server loop
- `RemoteCtrl/RemoteCtrl/ScreenCapture.*`: PNG capture helper

## Important viewer files

- `RemoteCtrl/RemoteClient/RemoteClient.cpp`: MFC app entry
- `RemoteCtrl/RemoteClient/RemoteClientDlg.*`: connect/auth/status UI
- `RemoteCtrl/RemoteClient/CClientSocket.*`: persistent client socket + receive loop
- `RemoteCtrl/RemoteClient/CWatchDialog.*`: read-only watch window

## Testing

- Use `PacketTests` for packet parser regression coverage.
- Preferred manual smoke path:
  1. Run `AssistHost`
  2. Run `AssistViewer`
  3. Submit a wrong code, then a correct code
  4. Deny once, allow once
  5. End the session from both host and viewer

## Legacy reference

`RemoteCtrl/RemoteControlSystemDesign.mdj` is legacy study material only. It no longer describes the active implementation path.
