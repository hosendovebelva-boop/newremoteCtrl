# Repository Guidelines

## Project Structure & Module Organization
- `RemoteCtrl.sln` is the Visual Studio 2022 solution containing two MFC projects.
- `RemoteCtrl/` is the server (target machine). Key files include `RemoteCtrl.cpp`, `ServerSocket.*`, and the `.rc` resources.
- `RemoteClient/` is the client GUI. Look for `RemoteClient.cpp`, `RemoteClientDlg.*`, and `res/` assets.
- `x64/` and `*/x64/Debug/` contain build outputs; avoid committing generated artifacts.

## Build, Test, and Development Commands
- `msbuild RemoteCtrl.sln /p:Configuration=Debug /p:Platform=x64` builds debug binaries.
- `msbuild RemoteCtrl.sln /p:Configuration=Release /p:Platform=x64` builds release binaries.
- Run locally by launching `x64/Debug/RemoteCtrl.exe` (server) and `x64/Debug/RemoteClient.exe` (client).

## Coding Style & Naming Conventions
- C++/MFC style with `.h/.cpp` pairs and PascalCase types (e.g., `CServerSocket`).
- Use precompiled headers via `pch.h`/`pch.cpp` and include `pch.h` first in `.cpp` files.
- Keep resource edits in `.rc` and `resource.h` in sync when adding dialogs, icons, or strings.

## Testing Guidelines
- No automated test framework is present. Use manual smoke tests against the client/server pair.
- When adding protocol commands, verify packet parsing and checksum handling using the `Dump()` helper in `RemoteCtrl.cpp`.

## Commit & Pull Request Guidelines
- Existing commits use short, Chinese, sentence-style messages describing the change. Follow this pattern for consistency.
- PRs should include a concise summary, test notes (manual steps), and screenshots for UI changes.

## Configuration & Security Notes
- The server listens on TCP port 9527. Document any port or protocol changes and update firewall guidance if needed.
