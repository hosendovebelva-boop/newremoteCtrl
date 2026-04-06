# RemoteCtrl Project Guide

> This document gives an English overview of the RemoteCtrl codebase, including architecture, protocol design, command flow, thread model, and current limitations.

## Overview

RemoteCtrl is a Windows remote control system built with a classic client-server architecture.

- `RemoteCtrl` is the controlled side. It runs on the target machine, listens on TCP port `9527`, receives commands, and performs actions locally.
- `RemoteClient` is the controller side. It provides an MFC UI for file browsing, screen monitoring, mouse control, file download, and machine lock/unlock.

Technology stack:

- Language: C++
- UI: MFC dialog-based applications
- Networking: Winsock 1.1 over TCP
- Imaging: `CImage`, GDI, screen capture and PNG encoding
- Toolchain: Visual Studio 2022, x64 target

## Repository Layout

```text
RemoteCtrl/
|-- RemoteCtrl.sln
|-- CLAUDE.md
|-- ProjectGuide.md
|-- RemoteControlSystemDesign.mdj
|-- RemoteCtrl/
|   |-- RemoteCtrl.cpp
|   |-- RemoteCtrl.h
|   |-- RemoteCtrl.rc
|   |-- ServerSocket.h / ServerSocket.cpp
|   |-- Packet.h
|   |-- Command.h / Command.cpp
|   |-- LockInfoDialog.h / LockInfoDialog.cpp
|   |-- EdoyunTool.h / EdoyunTool.cpp
|   |-- framework.h
|   |-- pch.h / pch.cpp
|   |-- resource.h
|   `-- targetver.h
`-- RemoteClient/
    |-- RemoteClient.cpp / RemoteClient.h
    |-- RemoteClientDlg.cpp / RemoteClientDlg.h
    |-- CClientSocket.cpp / CClientSocket.h
    |-- ClientController.cpp / ClientController.h
    |-- CWatchDialog.cpp / CWatchDialog.h
    |-- StatusDlg.cpp / StatusDlg.h
    |-- EdoyunTool.h
    |-- RemoteClient.rc
    |-- framework.h
    |-- pch.h / pch.cpp
    |-- resource.h
    `-- targetver.h
```

## Architecture

### Shared Protocol Layer

The protocol is duplicated on both sides:

- Server: `RemoteCtrl/Packet.h`
- Client: `RemoteClient/CClientSocket.h`

Both sides must stay in sync for packet layout and command semantics.

### Server Side

Main responsibilities:

- Accept incoming TCP connections.
- Parse packets from the client.
- Dispatch commands through `CCommand`.
- Perform local file, screen, mouse, and lock operations.

Core classes:

- `CServerSocket`: singleton TCP server and receive/send path.
- `CPacket`: packet parser/serializer.
- `CCommand`: command dispatcher and implementation.
- `CLockInfoDialog`: full-screen lock overlay.

### Client Side

Main responsibilities:

- Present the controller UI.
- Send commands to the server.
- Display file lists, progress, and screen data.
- Convert local UI input into remote actions.

Core classes:

- `CClientSocket`: singleton TCP client.
- `CClientController`: high-level command coordinator.
- `CRemoteClientDlg`: main file browsing and control UI.
- `CWatchDialog`: remote desktop display and mouse input.
- `CStatusDlg`: progress/status feedback.

## Protocol Summary

The application uses a custom little-endian binary packet format.

| Field | Size | Description |
|---|---:|---|
| Header | 2 bytes | Magic marker `0xFEFF` |
| Length | 4 bytes | Length from command field through checksum |
| Command | 2 bytes | Command ID |
| Data | N bytes | Variable-length payload |
| Checksum | 2 bytes | WORD sum/check value |

### Core Structures

`CPacket`

- Serializes requests and responses.
- Parses partial network buffers.
- Uses a consumed-length out parameter when parsing receive data.

`MOUSEEV`

- `nAction`: click, double-click, down, up, move
- `nButton`: left, right, middle, none
- `ptXY`: target coordinates

`FILEINFO`

- Signals whether the entry is valid, whether it is a directory, whether more entries follow, and the file name.

## Command Map

| Command | Meaning | Server handler |
|---:|---|---|
| 1 | Query drive list | `MakeDriverInfo()` |
| 2 | Query directory contents | `MakeDirectoryInfo()` |
| 3 | Run file | `RunFile()` |
| 4 | Download file | `DownloadFile()` |
| 5 | Mouse event | `MouseEvent()` |
| 6 | Capture and send screen | `SendScreen()` |
| 7 | Lock machine | `LockMachine()` |
| 8 | Unlock machine | `UnlockMachine()` |
| 9 | Delete file | `DeleteLocalFile()` |
| 1981 | Connection test | `TestConnect()` |

## Main Flows

### File Browsing

1. Client sends Command 1 to get available drives.
2. Server returns a comma-separated drive list.
3. Client populates the tree view.
4. When the user expands or clicks a node, the client sends Command 2 with the selected path.
5. Server enumerates the directory and returns multiple `FILEINFO` records.
6. Client updates the tree and file list controls.

### File Download

1. Client sends Command 4 with the remote file path.
2. Server replies with the file size first.
3. Server streams file chunks in subsequent packets.
4. Client writes chunks to a local file and updates state until complete.

### Remote Desktop Monitoring

1. Watch thread repeatedly requests Command 6.
2. Server captures the screen using GDI and encodes it through `CImage`.
3. Client receives image bytes, decodes them, and paints them in `CWatchDialog`.
4. Mouse input is converted from local control coordinates to remote screen coordinates.

### Machine Lock / Unlock

1. Client sends Command 7 to lock the machine.
2. Server launches `CLockInfoDialog`, hides the taskbar, and restricts cursor movement.
3. Client sends Command 8 to unlock.
4. Server releases the lock dialog and restores cursor/taskbar state.

## Thread Model

### Server

- Main thread: accepts connections and handles commands.
- Lock dialog thread: owns the lock UI while the machine is locked.

### Client

- UI thread: owns dialogs and control updates.
- Watch thread: polls screenshots.
- Download workflow: receives and writes streamed file data.

Important synchronization patterns:

- The client uses flags such as `m_isFull` and `m_isClosed` to coordinate watch dialog state.
- Some command dispatch paths bounce work back to the UI thread using Windows messages to avoid unsafe cross-thread UI access.

## Important Fixes Already Applied

Examples of fixes present in the codebase:

- 64-bit handle safety for `_findfirst` usage.
- Correct file download loop termination.
- Correct item selection after `HitTest` in the tree control.
- Better coordinate conversion for remote mouse events.
- Empty `OnOK()` override in the watch dialog to avoid accidental close on Enter.
- Better handling of receive return types and buffer consumption.

## Known Limitations

- Protocol definitions are duplicated on client and server.
- Most operations use short-lived socket workflows rather than a long-lived command channel.
- No encryption or authentication is implemented.
- The server currently handles a single controller at a time.
- Several project files were originally generated by Visual Studio and still reflect that structure.

## Practical Notes

- Build the solution with Visual Studio 2022.
- Debug and release outputs are generated under `x64/` and should not be committed.
- Resource and project files may use mixed encodings; preserving their original encoding matters when editing them.

## Suggested Next Improvements

- Extract shared protocol definitions into one common header/library.
- Add authentication and transport encryption.
- Move from polling-based screen requests to a more efficient streaming model.
- Remove generated build outputs from version control permanently.
- Add automated tests around packet parsing and command behavior where possible.
