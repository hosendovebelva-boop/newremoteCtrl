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

## 笔记协同

本项目与 Obsidian 笔记库关联，笔记记录项目的设计思路和实现细节。

| 项目 | 路径 |
|------|------|
| **笔记库** | `D:\obsidian\C++\6.项目\远控系统\` |
| **Skill** | `remote-ctrl-note` (在笔记库中使用) |

### 笔记原则

| 情况 | 处理方式 |
|------|---------|
| **新代码/新功能** | 完整展示代码 + 详细注释讲解 |
| **之前笔记讲过的代码** | 用 `[[wiki-link]]` 引用之前的笔记，不重复 |
| **关联的项目代码** | 引用项目文件路径 + 行号 |

### 已讲解代码索引

写新笔记前检查，避免重复讲解：

| 笔记 | 已讲解的内容 |
|------|-------------|
| 2.1 网络编程基本设计 | Winsock 初始化、socket/bind/listen/accept |
| 2.2 网络编程架构设计 | CServerSocket 单例模式、CHelper 自动释放 |
| 2.3 设计网络传输包协议 | CPacket 完整实现、协议格式、粘包处理、校验和 |
| 2.4 获取磁盘分区信息 | GetLogicalDriveStrings、命令处理框架 |

### 代码变更时

修改代码后，请检查是否需要更新笔记：
1. 函数重命名/移动 → 更新笔记中的代码和引用
2. 新增功能 → 新建笔记或扩展现有笔记
3. 更新上方"已讲解代码索引"表格

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

**Command IDs** (defined in `RemoteCtrl.cpp`):
- `1` - Get disk partition list (returns comma-separated drive letters like "C,D,E")
- `2` - Get directory file list (requires path in packet data)

### Key Patterns

- Singleton pattern for `CServerSocket` with manual memory management
- Precompiled headers (`pch.h`) for faster compilation
- MFC framework integration for both console and dialog applications

### Development Notes

- Code comments are primarily in Chinese (Simplified)
- The server main loop in `RemoteCtrl.cpp` is currently commented out during development
- `Dump()` function in `RemoteCtrl.cpp` outputs hex dumps to debug output for packet debugging

## 代码修改规范

修改代码时，必须添加注释说明变更，便于学习和追溯：

```cpp
// [原代码] int hfind = 0;
// [问题] 64位系统 _findfirst 返回 intptr_t(64位)，用 int(32位) 存储会截断句柄值
// [新代码] 使用 intptr_t 确保在 64 位系统正确存储句柄
intptr_t hfind = 0;
// [新代码结束]
```

| 标签 | 用途 |
|------|------|
| `[原代码]` | 展示被删除/替换的原始代码 |
| `[问题]` | 说明原代码的问题或修改原因 |
| `[新代码]` | 说明新代码的作用 |
| `[新代码结束]` | 标记新增代码块的结束位置 |
| `[修复]` | 说明修复方案 |
