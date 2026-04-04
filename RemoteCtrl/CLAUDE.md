# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build Commands

This is a Visual Studio 2022 (v17) solution. Build using:

```bash
# Command line build (requires VS Build Tools)
msbuild RemoteCtrl.sln /p:Configuration=Debug /p:Platform=x64
msbuild RemoteCtrl.sln /p:Configuration=Release /p:Platform=x64

# Or open RemoteCtrl.sln in Visual Studio and build from IDE
```

Output binaries are placed in `x64/Debug/` or `x64/Release/`.

---

## Note Collaboration

This project is linked to an Obsidian notebook, which records design concepts and implementation details.

| Item | Path |
|------|------|
| **Notebook** | `D:\obsidian\C++\6.项目\远控系统\` |
| **Skill** | `remote-ctrl-note` (Used within the notebook) |

### Skill Configuration

```json
{
  "skill": "D:/obsidian/C++/.claude/skills/remote-ctrl-note/skill.md"
}
```

### Note Principles

| Scenario | Handling Method |
|------|---------|
| **New Code/Features** | Full code display + detailed explanatory comments |
| **Previously Explained Code** | Reference via `[[wiki-link]]`; do not repeat |
| **Related Project Code** | Reference project file path + line numbers |

### Index of Explained Code

Check before writing new notes to avoid duplication:

| Note | Content Explained |
|------|-------------|
| 2.1 Basic Network Design | Winsock initialization, socket/bind/listen/accept |
| 2.2 Network Architecture Design | CServerSocket Singleton, CHelper automatic release |
| 2.3 Network Protocol Design | CPacket implementation, protocol format, sticky packets, checksum |
| 2.4 Get Disk Partitions | GetLogicalDriveStrings, command processing framework |
| 2.5 Get Directory List | _findfirst/_findnext, FILEINFO structure, Command 2 |
| 2.6 Remote Desktop | GetDC/BitBlt, CImage PNG encoding, screenshot workflow |
| 2.7 Mouse Remote Control | MOUSEEV structure, SetCursorPos/mouse_event, coordinate conversion |
| 2.8 File Download | Block transfer, thread mode, fopen/fread/fwrite |
| 2.9 Machine Lock/Unlock | CLockInfoDialog, ClipCursor, ShowCursor, taskbar hiding |
| 5.5 Debug: Crashes & Data Loss | _findfirst handle truncation fix, DealCommand buffer consumption, x64 debugging |

### Implemented Features List

| Feature | Command | Server Function | Client Entry Point |
|------|------|-----------|-----------|
| Get Disks | Cmd 1 | `MakeDriverInfo()` | File Info Button |
| Get Directory | Cmd 2 | `MakeDirectoryInfo()` | Tree Node Double-click |
| Run File | Cmd 3 | `RunFile()` | Context Menu → Run |
| Download File | Cmd 4 | `DownloadFile()` | Context Menu → Download |
| Mouse Control | Cmd 5 | `MouseEvent()` | CWatchDialog Mouse Events |
| Screenshot | Cmd 6 | `SendScreen()` | CWatchDialog Timer |
| Lock Machine | Cmd 7 | `LockMachine()` | Lock Button |
| Unlock Machine | Cmd 8 | `UnlockMachine()` | Unlock Button |
| Delete File | Cmd 9 | `DeleteLocalFile()` | Context Menu → Delete |
| Connection Test | Cmd 1981 | - | Test Button |

### During Code Changes

After modifying code, please check if notes need updating:
1. Function renamed/moved → Update code and references in notes.
2. New feature added → Create new note or expand existing ones.
3. Update the "Index of Explained Code" table above.

---

## Architecture

Windows Remote Control System based on Client-Server architecture, implemented using MFC and Winsock.

### System Diagram

```
┌─────────────────────────────────────────────────────────────────────────┐
│                           RemoteClient (Controller)                     │
│  ┌─────────────────┐  ┌─────────────────┐  ┌─────────────────┐         │
│  │ CRemoteClientDlg│  │  CWatchDialog   │  │   CStatusDlg    │         │
│  │ Main UI/File Nav│  │ Desktop/Mouse   │  │ Progress Display │         │
│  └────────┬────────┘  └────────┬────────┘  └─────────────────┘         │
│           │                    │                                        │
│           └──────────┬─────────┘                                        │
│                      ▼                                                  │
│           ┌─────────────────────┐                                       │
│           │   CClientSocket     │  (Singleton, TCP Client)              │
│           │   CPacket (Protocol)│                                       │
│           └──────────┬──────────┘                                       │
└──────────────────────┼──────────────────────────────────────────────────┘
                       │ TCP Port 9527
                       ▼
┌──────────────────────┼──────────────────────────────────────────────────┐
│           ┌──────────┴──────────┐                                       │
│           │   CServerSocket     │  (Singleton, TCP Server)              │
│           │   CPacket (Parsing) │                                       │
│           └──────────┬──────────┘                                       │
│                      ▼                                                  │
│           ┌─────────────────────┐                                       │
│           │   ExcuteCommand()   │  Command Dispatcher                   │
│           └──────────┬──────────┘                                       │
│                      ▼                                                  │
│  ┌────────┬────────┬────────┬────────┬────────┬────────┬────────┐      │
│  │ Disk   │ Dir    │ Run    │ Download│ Mouse  │ Screen │ Lock   │      │
│  │ Cmd 1  │ Cmd 2  │ Cmd 3  │ Cmd 4   │ Cmd 5  │ Cmd 6  │ Cmd 7-8│      │
│  └────────┴────────┴────────┴────────┴────────┴────────┴────────┘      │
│                           RemoteCtrl (Controlled)                       │
└─────────────────────────────────────────────────────────────────────────┘
```

### Projects

| Project | Path | Description |
|-----|------|------|
| **RemoteCtrl** | `RemoteCtrl/` | Server (Controlled), runs on target machine, executes commands |
| **RemoteClient** | `RemoteClient/` | Client (Controller), MFC Dialog app, controls the server |

### File Structure

```
RemoteCtrl/
├── RemoteCtrl/              # Server
│   ├── RemoteCtrl.cpp       # Main entry, 9 command handlers
│   ├── ServerSocket.h/cpp   # TCP Server, CPacket protocol
│   ├── LockInfoDialog.* # Lock screen UI overlay
│   └── RemoteCtrl.vcxproj
│
├── RemoteClient/            # Client
│   ├── RemoteClientDlg.* # Main UI: File browsing, thread management
│   ├── CWatchDialog.* # Remote desktop display, mouse control
│   ├── CClientSocket.h      # TCP Client, CPacket protocol
│   ├── StatusDlg.* # Progress dialog
│   └── RemoteClient.vcxproj
│
├── RemoteCtrl.sln           # Solution File
└── CLAUDE.md                # This document
```

---

## Network Protocol

### Data Packet Format

Binary protocol, Little-endian:

```
┌────────┬────────┬────────┬─────────────┬──────────┐
│ Header │ Length │Command │    Data     │ Checksum │
│  2B    │  4B    │  2B    │   N bytes   │   2B     │
└────────┴────────┴────────┴─────────────┴──────────┘
```

| Field | Size | Description |
|------|------|------|
| Header | 2 bytes | Magic number `0xFEFF`, identifies start of packet |
| Length | 4 bytes | Data length = sizeof(data) + 4 (includes Command field) |
| Command | 2 bytes | Operation type ID |
| Data | N bytes | Variable length payload |
| Checksum | 2 bytes | WORD sum of all Data bytes |

**Total Packet Size**: `nLength + 6` bytes

### Protocol Class

```cpp
class CPacket {
public:
    WORD sHead;         // 0xFEFF
    DWORD nLength;      // Data length
    WORD sCmd;          // Command ID
    std::string strData;// Data payload
    WORD sSum;          // Checksum

    // Construct from raw bytes (Parsing)
    CPacket(const BYTE* pData, size_t& nSize);
    // Construct from command and data (Sending)
    CPacket(WORD nCmd, const BYTE* pData, size_t nSize);
    // Serialization
    int Size() const;
    const char* Data();
};
```

---

## Core Data Structures

### Mouse Event (MOUSEEV)

```cpp
typedef struct MouseEvent {
    WORD nAction;    // Action: 0=Click, 1=Double-click, 2=Down, 3=Up
    WORD nButton;    // Button: 0=Left, 1=Right, 2=Middle, 4=None, 8=Move
    POINT ptXY;      // Coordinates (x, y)
} MOUSEEV;
```

### File Information (FILEINFO)

```cpp
typedef struct file_info {
    BOOL IsInvalid;       // Invalid path flag
    BOOL IsDirectory;     // Directory flag (-1=Unknown, 0=File, 1=Dir)
    BOOL HasNext;         // Continuation flag (0=None, 1=More follows)
    char szFileName[256]; // File/Directory name
} FILEINFO;
```

---

## Core Components

### CServerSocket (Server)
Location: `RemoteCtrl/ServerSocket.h`
- Singleton pattern.
- Handles Winsock initialization, listening (Port 9527), and command parsing.

### CClientSocket (Client)
Location: `RemoteClient/CClientSocket.h`
- Singleton pattern.
- Handles server connection and receiving responses with a 4MB buffer.

### CRemoteClientDlg (Main Dialog)
- Manages file tree/list controls and right-click menu actions.
- Handles background threads for file downloads and screen monitoring.

---

## Thread Model

### Server Threads
- **Main Thread**: Blocked on `AcceptClient()` and `DealCommand()`.
- **Lock Thread**: Created via `_beginthreadex()` for the lock screen UI.

### Client Threads
- **Main Thread**: UI event processing.
- **Watch Thread**: Every 50ms sends Command 6 (Screen Request) and updates the CImage buffer.
- **Download Thread**: Streams file data and updates progress UI.

---

## Fixed Bugs

| Bug | Cause | Fix |
|-----|------|------|
| 64-bit handle truncation | `_findfirst` returned `intptr_t`, stored in `int` | Changed to `intptr_t hfind` |
| File download loop | Incorrect `rlen > 1024` logic | Changed to `rlen >= 1024` |
| Coordination offset | Conversion didn't account for scaling | Implemented `UserPoint2RemoteScreenPoint` |

---

## Code Modification Standards

When modifying code, you must add comments explaining the change:

```cpp
// [Original Code] int hfind = 0;
// [Issue] _findfirst returns intptr_t (64-bit) in x64, using int (32-bit) causes truncation
// [New Code] Using intptr_t to ensure handle is stored correctly on 64-bit systems
intptr_t hfind = 0;
// [New Code End]
```