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

## Architecture

This is a Windows remote control application with client-server architecture using MFC and Winsock.

### Projects

- **RemoteCtrl/** - Server component (被控端). MFC console application that runs on the target machine, listens for connections and executes commands.
- **RemoteClient/** - Client component (控制端). MFC dialog-based GUI application that connects to and controls the server.

### Core Components

**CServerSocket** (`RemoteCtrl/ServerSocket.h`): Singleton TCP server socket manager
- Initializes Winsock environment (version 1.1)
- Binds to port 9527 on all interfaces
- Handles client connections and command processing
- Uses `CHelper` nested class for automatic resource cleanup

**CPacket** (`RemoteCtrl/ServerSocket.h`): Custom binary protocol packet
- Packet format: `[Header 2B][Length 4B][Command 2B][Data...][Checksum 2B]`
- Header magic: `0xFEFF`
- Includes checksum validation for data integrity
- Supports both construction from raw bytes and serialization

### Network Protocol

- TCP socket communication
- Server listens on port 9527
- Command-based protocol where `sCmd` field identifies the operation
- Data payload in `strData` with checksum verification

### Key Patterns

- Singleton pattern for `CServerSocket` with manual memory management
- Precompiled headers (`pch.h`) for faster compilation
- MFC framework integration for both console and dialog applications
