# Repository Guidelines

## Project Structure & Module Organization
`RemoteCtrl.sln` contains two Visual Studio C++ projects:

- `RemoteCtrl/`: server-side application. Core networking and command handling live in `ServerSocket.*`, `Command.*`, and `Packet.h`.
- `RemoteClient/`: MFC client UI. Dialog classes live in `*Dlg.*`, controller/socket logic in `ClientController.*` and `CClientSocket.*`, and assets in `RemoteClient/res/`.

Shared build scaffolding such as `pch.h`, `framework.h`, `.rc`, and `resource.h` stays inside each project directory. Top-level design artifacts (`*.mdj`, project notes, screenshots) are reference material, not runtime code.


## Build, Test, and Development Commands
Use Visual Studio 2022 with the `v143` toolset and Windows SDK 10.0.

```powershell
msbuild .\RemoteCtrl.sln /p:Configuration=Debug /p:Platform=x64
msbuild .\RemoteCtrl.sln /p:Configuration=Release /p:Platform=x64
devenv .\RemoteCtrl.sln
```

The first two commands build both projects in Debug or Release. `devenv` opens the full solution for interactive debugging, resource editing, and MFC designer work.

## Coding Style & Naming Conventions
Follow the existing C++/MFC style:

- Use tabs for indentation and keep braces on their own lines.
- Keep class names in `PascalCase` with the existing `C` prefix for MFC-style types, for example `CClientController`.
- Use member prefixes already present in the codebase: `m_` for members, `n` for integers, `b` for booleans, `str` for strings, and `p` for pointers.
- Prefer one class per header/source pair and keep resource identifiers in `resource.h`.

There is no formatter config in the repo, so match nearby code before submitting changes.

## Testing Guidelines
There is currently no automated test project or test framework checked in. Validate changes by building both `Debug|x64` and `Release|x64`, then smoke-test the client/server flow manually: connect, issue commands, verify file operations, and confirm UI dialogs still behave correctly.

## Commit & Pull Request Guidelines
Recent commits use short, task-focused summaries, often numbered to group related changes. Keep commits scoped and descriptive, for example `1 refactor client controller 2 fix download dialog state`.

Pull requests should include:

- a brief summary of behavior changes
- affected project(s): `RemoteCtrl`, `RemoteClient`, or both
- manual verification steps
- screenshots for UI or dialog changes
- linked issue or design note when available

## File Location Link Output Template
- Standard clickable format: `[absolute_path:line_number](/absolute_path#Lline_number)`; Example: `[{{ABS_PATH}}:{{LINE}}](/{{ABS_PATH}}#L{{LINE}})`
- Path requirements: Absolute path, use `/`, no quotes.

