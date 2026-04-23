# ScreenShare Project Guide

## Overview

This codebase is a visible, consent-gated LAN screen sharing sample.

- `ScreenShareHost` runs on the machine being viewed.
- `ScreenShareViewer` runs on the machine requesting access.
- The viewer submits a 6-digit session code.
- The host user must allow the request before frames are sent.
- Frames are requested on a fixed 500 ms cadence.

The active implementation supports a single viewer and read-only screenshots only.

## Shared protocol

Shared headers live at the solution root:

- `RemoteCtrl/ScreenShareProtocol.h`
- `RemoteCtrl/SharedPacket.h`

Command IDs:

| Command | Direction | Meaning |
|---|---|---|
| `1001 SubmitSessionCode` | Viewer -> Host | Submit exactly 6 ASCII digits |
| `1002 SessionStatus` | Host -> Viewer | Return auth/session state |
| `6 SendScreen` | Viewer -> Host, Host -> Viewer | Empty request, PNG response |
| `1003 EndSession` | Either direction | End the current session |

Session status payload values:

| Value | Meaning |
|---|---|
| `0` | Bad code |
| `1` | Code accepted, waiting for host consent |
| `2` | Host denied the request |
| `3` | Host granted the request |
| `4` | Host already has an active viewer |
| `5` | Session ended |

## Host architecture

Files:

- `RemoteCtrl/RemoteCtrl/RemoteCtrl.cpp`
- `RemoteCtrl/RemoteCtrl/HostMainDlg.*`
- `RemoteCtrl/RemoteCtrl/ServerSocket.*`
- `RemoteCtrl/RemoteCtrl/ConsentDialog.*`
- `RemoteCtrl/RemoteCtrl/ShareBannerWnd.*`
- `RemoteCtrl/RemoteCtrl/ScreenCapture.*`

Behavior:

- Starts Winsock at app scope
- Binds `INADDR_ANY:9527`
- Shows local IPv4 addresses and the current 6-digit session code
- Posts consent requests back to the UI thread
- Shows a topmost banner while a session is active
- Exposes tray actions for show, stop, and exit

## Viewer architecture

Files:

- `RemoteCtrl/RemoteClient/RemoteClient.cpp`
- `RemoteCtrl/RemoteClient/RemoteClientDlg.*`
- `RemoteCtrl/RemoteClient/CClientSocket.*`
- `RemoteCtrl/RemoteClient/CWatchDialog.*`

Behavior:

- Connects to the host once per session
- Sends the session code immediately after connect
- Waits for `SessionStatus`
- Opens a modeless watch window after consent is granted
- Requests the next frame every 500 ms
- Sends `EndSession` when the watch window is closed locally

## Packet tests

`RemoteCtrl/PacketTests/PacketTests.cpp` covers:

- Complete packet parse
- Fragmented header
- Fragmented payload
- Two packets in one buffer
- Bad checksum
- Empty payload

## Legacy note

`RemoteCtrl/RemoteControlSystemDesign.mdj` and older notes refer to the previous remote-control study code. They are not the source of truth for the active implementation anymore.
