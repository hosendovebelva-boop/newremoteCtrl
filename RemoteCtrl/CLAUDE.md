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

### Skill 配置

```json
{
  "skill": "D:/obsidian/C++/.claude/skills/remote-ctrl-note/skill.md"
}
```

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
| 2.5 获取目录文件列表 | _findfirst/_findnext、FILEINFO 结构、Command 2 |
| 2.6 远程桌面实现 | GetDC/BitBlt、CImage PNG编码、屏幕截图流程 |
| 2.7 鼠标远程控制 | MOUSEEV 结构、SetCursorPos/mouse_event、坐标转换 |
| 2.8 文件下载实现 | 分块传输、线程模式、fopen/fread/fwrite |
| 2.9 机器锁定解锁 | CLockInfoDialog、ClipCursor、ShowCursor、任务栏隐藏 |

### 已实现功能清单

| 功能 | 命令 | 服务端函数 | 客户端入口 |
|------|------|-----------|-----------|
| 获取磁盘分区 | Cmd 1 | `MakeDriverInfo()` | 文件信息按钮 |
| 获取目录列表 | Cmd 2 | `MakeDirectoryInfo()` | 树节点双击 |
| 运行文件 | Cmd 3 | `RunFile()` | 右键菜单→运行 |
| 下载文件 | Cmd 4 | `DownloadFile()` | 右键菜单→下载 |
| 鼠标控制 | Cmd 5 | `MouseEvent()` | CWatchDialog 鼠标事件 |
| 屏幕截图 | Cmd 6 | `SendScreen()` | CWatchDialog 定时器 |
| 锁定机器 | Cmd 7 | `LockMachine()` | 锁定按钮 |
| 解锁机器 | Cmd 8 | `UnlockMachine()` | 解锁按钮 |
| 删除文件 | Cmd 9 | `DeleteLocalFile()` | 右键菜单→删除 |
| 连接测试 | Cmd 1981 | - | 测试按钮 |

### 代码变更时

修改代码后，请检查是否需要更新笔记：
1. 函数重命名/移动 → 更新笔记中的代码和引用
2. 新增功能 → 新建笔记或扩展现有笔记
3. 更新上方"已讲解代码索引"表格

## Architecture

Windows 远程控制系统，采用 Client-Server 架构，基于 MFC 和 Winsock 实现。

### 系统架构图

```
┌─────────────────────────────────────────────────────────────────────────┐
│                           RemoteClient (控制端)                          │
│  ┌─────────────────┐  ┌─────────────────┐  ┌─────────────────┐         │
│  │ CRemoteClientDlg│  │  CWatchDialog   │  │   CStatusDlg    │         │
│  │   主界面/文件浏览  │  │  远程桌面/鼠标控制 │  │    进度显示     │         │
│  └────────┬────────┘  └────────┬────────┘  └─────────────────┘         │
│           │                    │                                        │
│           └──────────┬─────────┘                                        │
│                      ▼                                                  │
│           ┌─────────────────────┐                                       │
│           │   CClientSocket     │  (Singleton, TCP Client)              │
│           │   CPacket (协议封装)  │                                       │
│           └──────────┬──────────┘                                       │
└──────────────────────┼──────────────────────────────────────────────────┘
                       │ TCP Port 9527
                       ▼
┌──────────────────────┼──────────────────────────────────────────────────┐
│           ┌──────────┴──────────┐                                       │
│           │   CServerSocket     │  (Singleton, TCP Server)              │
│           │   CPacket (协议解析)  │                                       │
│           └──────────┬──────────┘                                       │
│                      ▼                                                  │
│           ┌─────────────────────┐                                       │
│           │   ExcuteCommand()   │  命令分发器                            │
│           └──────────┬──────────┘                                       │
│                      ▼                                                  │
│  ┌────────┬────────┬────────┬────────┬────────┬────────┬────────┐      │
│  │ 磁盘   │ 目录   │ 运行   │ 下载   │ 鼠标   │ 屏幕   │ 锁定   │      │
│  │ Cmd 1 │ Cmd 2 │ Cmd 3 │ Cmd 4 │ Cmd 5 │ Cmd 6 │ Cmd 7-8│      │
│  └────────┴────────┴────────┴────────┴────────┴────────┴────────┘      │
│                           RemoteCtrl (被控端)                            │
└─────────────────────────────────────────────────────────────────────────┘
```

### Projects

| 项目 | 路径 | 描述 |
|-----|------|------|
| **RemoteCtrl** | `RemoteCtrl/` | 服务端 (被控端)，运行在目标机器，监听连接并执行命令 |
| **RemoteClient** | `RemoteClient/` | 客户端 (控制端)，MFC 对话框应用，用于控制服务端 |

### 文件结构

```
RemoteCtrl/
├── RemoteCtrl/              # Server (被控端)
│   ├── RemoteCtrl.cpp       # 主入口，9种命令处理器
│   ├── ServerSocket.h/cpp   # TCP服务器，CPacket协议
│   ├── LockInfoDialog.*     # 锁屏UI覆盖层
│   └── RemoteCtrl.vcxproj
│
├── RemoteClient/            # Client (控制端)
│   ├── RemoteClientDlg.*    # 主UI：文件浏览，线程管理
│   ├── CWatchDialog.*       # 远程桌面显示，鼠标控制
│   ├── CClientSocket.h      # TCP客户端，CPacket协议
│   ├── StatusDlg.*          # 进度对话框
│   └── RemoteClient.vcxproj
│
├── RemoteCtrl.sln           # Solution 文件
└── CLAUDE.md                # 本文档
```

---

## 网络协议

### 数据包格式

二进制协议，小端序 (Little-endian)：

```
┌────────┬────────┬────────┬─────────────┬──────────┐
│ Header │ Length │Command │    Data     │ Checksum │
│  2B    │  4B    │  2B    │   N bytes   │   2B     │
└────────┴────────┴────────┴─────────────┴──────────┘
```

| 字段 | 大小 | 说明 |
|------|------|------|
| Header | 2 bytes | 魔数 `0xFEFF`，标识包起始 |
| Length | 4 bytes | 数据长度 = sizeof(data) + 4 (含 Command 字段) |
| Command | 2 bytes | 操作类型 ID |
| Data | N bytes | 可变长度负载 |
| Checksum | 2 bytes | 所有 Data 字节之和 (WORD) |

**总包大小**: `nLength + 6` 字节

### 协议类

```cpp
class CPacket {
public:
    WORD sHead;         // 0xFEFF
    DWORD nLength;      // 数据长度
    WORD sCmd;          // 命令ID
    std::string strData;// 数据负载
    WORD sSum;          // 校验和

    // 从原始字节构造 (解析)
    CPacket(const BYTE* pData, size_t& nSize);
    // 从命令和数据构造 (发送)
    CPacket(WORD nCmd, const BYTE* pData, size_t nSize);
    // 序列化
    int Size() const;
    const char* Data();
};
```

### 命令列表

| ID | 名称 | 方向 | 描述 | 请求数据 | 响应数据 |
|----|------|------|------|----------|----------|
| 1 | 获取磁盘 | C→S | 获取所有磁盘分区 | 无 | 逗号分隔的盘符 "C,D,E" |
| 2 | 获取目录 | C→S | 列出目录内容 | 路径字符串 | 多个 FILEINFO 结构 |
| 3 | 运行文件 | C→S | 执行指定文件 | 文件路径 | 无 |
| 4 | 下载文件 | C→S | 下载文件 | 文件路径 | 文件大小(8B) + 数据块 |
| 5 | 鼠标事件 | C→S | 远程鼠标控制 | MOUSEEV 结构 | 无 |
| 6 | 发送屏幕 | C→S | 截取并发送屏幕 | 无 | PNG 图像字节流 |
| 7 | 锁定机器 | C→S | 禁用服务端输入 | 无 | 无 |
| 8 | 解锁机器 | C→S | 恢复服务端输入 | 无 | 无 |
| 9 | 删除文件 | C→S | 删除指定文件 | 文件路径 | 无 |
| 1981 | 测试连接 | 双向 | 连接测试/心跳 | 无 | 无 |

---

## 核心数据结构

### 鼠标事件 (MOUSEEV)

```cpp
typedef struct MouseEvent {
    WORD nAction;    // 动作: 0=单击, 1=双击, 2=按下, 3=释放
    WORD nButton;    // 按键: 0=左键, 1=右键, 2=中键, 4=无按键, 8=移动
    POINT ptXY;      // 坐标 (x, y)
} MOUSEEV;
```

### 文件信息 (FILEINFO)

```cpp
typedef struct file_info {
    BOOL IsInvalid;       // 路径无效标志
    BOOL IsDirectory;     // 目录标志 (-1=未知, 0=文件, 1=目录)
    BOOL HasNext;         // 后续标志 (0=无更多, 1=有后续)
    char szFileName[256]; // 文件/目录名
} FILEINFO;
```

---

## 核心组件

### CServerSocket (服务端)

位置: `RemoteCtrl/ServerSocket.h`

```cpp
class CServerSocket {
    static CServerSocket* m_instance;  // 单例实例
    SOCKET m_sock;                     // 监听套接字
    SOCKET m_client;                   // 客户端套接字

    class CHelper { /* 自动清理辅助类 */ };

public:
    static CServerSocket* getInstance();  // 获取单例
    bool InitSockEnv();    // 初始化 Winsock
    bool InitSocket();     // 创建并绑定套接字 (端口 9527)
    bool AcceptClient();   // 接受客户端连接
    int  DealCommand();    // 接收并解析命令
    bool Send(CPacket& pack);  // 发送数据包
};
```

### CClientSocket (客户端)

位置: `RemoteClient/CClientSocket.h`

```cpp
class CClientSocket {
    static CClientSocket* m_instance;  // 单例实例
    SOCKET m_sock;                     // 连接套接字
    std::vector<char> m_buffer;        // 4MB 接收缓冲区
    CPacket m_packet;                  // 当前数据包

public:
    static CClientSocket* getInstance();
    bool InitSocket(DWORD nIP, short nPort);  // 连接服务器
    int  DealCommand();     // 接收并解析响应
    bool Send(CPacket& pack);
    CPacket& GetPacket();   // 获取当前数据包
};
```

### CRemoteClientDlg (主对话框)

位置: `RemoteClient/RemoteClientDlg.h`

职责:
- IP/端口输入控件
- 文件树浏览 (Tree Control)
- 文件列表显示 (List Control)
- 右键菜单 (下载/删除/运行)
- 线程管理 (文件下载、屏幕监控)

关键成员:
```cpp
CTreeCtrl m_Tree;        // 目录树
CListCtrl m_List;        // 文件列表
CWatchDialog* m_dlgWatch;// 监控对话框
CImage* m_image;         // 屏幕图像缓存
```

### CWatchDialog (监控对话框)

位置: `RemoteClient/CWatchDialog.h`

职责:
- 远程桌面显示 (CImage + StretchBlt)
- 鼠标事件捕获与转换
- 锁定/解锁按钮
- 定时刷新 (45ms Timer)

关键成员:
```cpp
CImage* m_image;         // 屏幕图像指针
int m_nObjWidth;         // 远程屏幕宽度
int m_nObjHeight;        // 远程屏幕高度
bool m_isClosed;         // 关闭标志
```

坐标转换:
```cpp
CPoint UserPoint2RemoteScreenPoint(CPoint& point, CRect& rect) {
    // 将控件坐标转换为远程屏幕坐标
    // 考虑缩放比例和偏移
}
```

---

## 数据流

### 远程桌面监控流程

```
Client (Watch Thread)              Server
     │                               │
     ├──Send Command 6 (无数据)─────→│ SendScreen()
     │                               ├─ GetDC(NULL) 获取屏幕DC
     │                               ├─ BitBlt 截取屏幕
     │                               ├─ 编码为 PNG (IStream)
     │                               └─ Send PNG 数据
     │                               │
     │←──Receive PNG 数据────────────┤
     ├─ 加载到 CImage
     ├─ StretchBlt 缩放显示
     └─ 循环 (每 50ms)
```

### 文件下载流程

```
Client                              Server
  │                                   │
  ├──Send Command 4 (文件路径)──────→│ DownloadFile()
  │                                   ├─ fopen(path, "rb")
  │                                   ├─ 获取文件大小
  │                                   ├─ Send 8字节大小头
  │                                   └─ 循环: 读取1024B块并发送
  │                                   │
  │←──Receive 文件大小 (8B)──────────┤
  │←──Receive 多个 Command 4 响应────┤
  └─ 写入本地文件
```

### 鼠标控制流程

```
Client (Watch Dialog)              Server
     │                               │
     ├─ 用户点击显示区域              │
     ├─ 转换坐标 (缩放计算)           │
     ├──Send Command 5 (MOUSEEV)───→│ MouseEvent()
     │                               ├─ SetCursorPos(x, y)
     │                               └─ mouse_event(action)
     └─ 等待下一个事件
```

---

## 线程模型

### 服务端线程

```
Main Thread:
  while(true) {
      AcceptClient();   // 阻塞等待连接
      DealCommand();    // 处理单个命令
      CloseClient();    // 断开连接
  }

Lock Thread (threadLockDlg):
  - _beginthreadex() 创建
  - GetMessage() 消息循环
  - 等待 'A' 键解锁
```

### 客户端线程

```
Main Thread:
  - UI 事件处理
  - 定时器刷新显示

Watch Thread (threadWatchData):
  - 每 50ms 发送 Command 6
  - 接收 PNG，更新缓存
  - m_isClosed = false 时持续

Download Thread (threadDownFile):
  - 流式接收文件数据
  - 显示进度对话框
```

---

## 设计模式

| 模式 | 应用 | 位置 |
|------|------|------|
| **Singleton** | CServerSocket, CClientSocket | ServerSocket.h, CClientSocket.h |
| **Command** | 命令 ID (1-9, 1981) 分发 | RemoteCtrl.cpp |
| **Observer** | Timer + 异步线程更新 UI | CWatchDialog |
| **Binary Protocol** | CPacket 二进制封包 | 协议层 |

---

## 已修复的 Bug

| Bug | 原因 | 修复 |
|-----|------|------|
| 64位句柄截断 | `_findfirst` 返回 `intptr_t`，用 `int` 存储 | 改用 `intptr_t hfind` |
| 文件下载无限循环 | `rlen > 1024` 判断错误 | 改为 `rlen >= 1024` |
| 连接断开 | 无条件调用 `CloseSocket()` | 移除不必要的关闭调用 |
| 监视界面崩溃 | 反复打开时资源未正确释放 | 正确管理 CImage 生命周期 |
| 回车键关闭对话框 | 默认按钮行为 | 重写 PreTranslateMessage |
| 鼠标坐标偏移 | 坐标转换未考虑缩放和偏移 | UserPoint2RemoteScreenPoint |

---

## 已知设计问题

| 问题 | 描述 | 影响 |
|------|------|------|
| **紧耦合** | 网络通信直接调用 UI 方法 | 难以测试和维护 |
| **单客户端** | 服务端一次只能接受一个连接 | 阻塞模式限制 |
| **无加密** | 明文 TCP 传输 | 安全风险 |
| **无认证** | 任何客户端都可连接 9527 端口 | 安全风险 |
| **缓冲区限制** | 4MB 缓冲区可能溢出 | 大文件传输问题 |

---

## Windows API 依赖

| 模块 | API | 用途 |
|------|-----|------|
| **Winsock** | WSAStartup, socket, bind, listen, accept, connect, recv, send | 网络通信 |
| **GDI+/CImage** | GetDC, BitBlt, StretchBlt, CImage::Save/Load | 屏幕截图与显示 |
| **MFC** | CDialog, CTreeCtrl, CListCtrl, CEdit | UI 框架 |
| **Win32** | SetCursorPos, mouse_event, ClipCursor, ShowWindow | 鼠标/窗口控制 |
| **文件系统** | _findfirst, _findnext, fopen, fread, fwrite | 文件操作 |

---

## 开发注意事项

- 代码注释主要使用简体中文
- `Dump()` 函数输出十六进制调试信息
- 修改代码时遵循下方的"代码修改规范"
- 新功能完成后更新"已讲解代码索引"

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
