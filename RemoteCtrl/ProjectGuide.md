# RemoteAssist Project Guide

## Overview

This codebase is a visible, consent-gated LAN remote-assistance sample.

- `AssistHost` runs on the machine being viewed.
- `AssistViewer` runs on the machine requesting access.
- The viewer sends a `Hello` handshake with a 6-digit session code and helper display name.
- The host user must allow the request before frames are sent.
- Frames are requested on a fixed 500 ms cadence.

The active implementation supports a single viewer and read-only screenshots only.

## Shared protocol

Shared headers live at the solution root:

- `RemoteCtrl/ScreenShareProtocol.h`
- `RemoteCtrl/SharedPacket.h`

Command ids:

| Command | Direction | Meaning |
|---|---|---|
| `1001 Hello` | Viewer -> Host | Send `<6 digits>\n<helper name>` in UTF-8 |
| `1002 ConsentResult` | Host -> Viewer | Return auth/consent state |
| `6 FrameRequest` | Viewer -> Host, Host -> Viewer | Empty request, PNG response |

Consent result payload values:

| Value | Meaning |
|---|---|
| `0` | Bad code |
| `1` | Code accepted, waiting for host consent |
| `2` | Host denied the request |
| `3` | Host granted the request |
| `4` | Host already has an active viewer |
| `5` | Host did not answer before the consent timeout |

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
- Parses the viewer `Hello` payload before showing consent
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
- Sends the `Hello` payload immediately after connect
- Waits for `ConsentResult`
- Opens a modeless watch window after consent is granted
- Requests the next frame every 500 ms
- Closes the TCP connection when the watch window is closed locally

## Safety rails

- `SessionPolicy.*` owns stream consent state for screen, microphone, and camera.
- Only screen is implemented; microphone and camera remain inactive placeholders.
- `ShareBannerWnd.*` shows the always-on-top host banner, screen/microphone indicators, remaining time, and the end-session affordance.
- `SessionLog.*` appends join, consent, start, end, extension, and expiration events to `%LOCALAPPDATA%\AssistHost\sessions.log`.
- Host sessions have a 60-minute default limit and can be extended locally in 15-minute increments.

## Packet tests

`RemoteCtrl/PacketTests/PacketTests.cpp` covers:

- `Hello` payload validation
- Complete packet parse
- Fragmented header
- Fragmented `Hello`
- Fragmented `FrameRequest` payload
- Two packets in one buffer
- Bad checksum
- Empty payload packet

## Legacy note

`RemoteCtrl/RemoteControlSystemDesign.mdj` and older notes refer to the previous remote-control study code. They are not the source of truth for the active implementation anymore.
