# MyComputerOnSteroid

**A native Windows launcher for the drives, folders, apps and files you choose.**

I keep returning to the same folders, applications and documents while moving
between projects. I started MyComputerOnSteroid to put those shortcuts beside my
drives in an Explorer-style view, using native C++ and the Windows Shell.

**Current status: repository and COM provider bootstrap.** The DLL exposes an
empty Shell folder and creates the system's default folder-view object. It does
not yet show your drives or favorites, register itself in Explorer, or provide a
usable launcher. There is no installer or application release yet.

[![Native build](https://github.com/umbertotechnopreneur/MyComputerOnSteroid/actions/workflows/build.yml/badge.svg)](https://github.com/umbertotechnopreneur/MyComputerOnSteroid/actions/workflows/build.yml)
[![MIT](https://img.shields.io/badge/license-MIT-4D91A8)](LICENSE)

## Build the starting point

Use Windows 11, PowerShell 7 and Visual Studio with Desktop development with C++,
the Windows SDK and CMake tools. Add the ARM64 C++ tools for that target.

```powershell
pwsh -NoProfile -File ./scripts/Build.ps1 -Architecture x64
pwsh -NoProfile -File ./scripts/Build.ps1 -Architecture arm64
```

The Debug DLL is written to
`artifacts/build/windows-<architecture>-debug/bin/MyComputerOnSteroid.Shell.dll`.
The build does not register the DLL, restart Explorer or change your settings.

See [build instructions](docs/BUILD.md), [current validation](docs/VALIDATION.md)
and [the architecture](docs/ARCHITECTURE.md).

## Launcher contract

These are the intended behaviors; they are **not implemented features** yet:

- Four groups: **Folders**, **Devices and drives**, **Apps**, and **Files**.
- Native icon, list and details views, with normal sorting and no manual ordering.
- Drag items into the launcher to save favorite references. Removing a favorite
  removes that reference; the original item remains. Drives cannot be removed.
- Explorer's context-menu behavior, with Windows 11's compact menu and
  **Show more options** subject to verification in the actual Explorer host.
- A launcher-only **Change icon** command and a native file picker that remembers
  its last location.

The requested image/acrylic background and small photo icon remain a feasibility
gate. No supported wallpaper API was established for a normal DefView. An image
concept would not prove that the standard Shell view can display it.

The implementation stays in C++20, Win32 and COM. The initial targets are
Windows 11 x64 and ARM64. [Open engineering work](.github/tasks/todo.md).

## Development

- [Contributing](CONTRIBUTING.md) and [agent instructions](AGENTS.md).
- [Completed work](.github/tasks/archive.md), [changelog](CHANGELOG.md) and
  [engineering lessons](.github/tasks/lessons.md).
- [AI contribution disclosure](AI_DEVELOPMENT.md) and [asset provenance](docs/IDENTITY.md).
- [Security reports](SECURITY.md), [support](SUPPORT.md) and [MIT license](LICENSE).

## About my work

I'm **Umberto Giacobbi**. I started and maintain MyComputerOnSteroid.
Find me at [umbertogiacobbi.biz](https://umbertogiacobbi.biz) or on
[LinkedIn](https://www.linkedin.com/in/umbertogiacobbi/).

More of my work: [WorkTrail](https://github.com/umbertotechnopreneur/WorkTrail),
[AgentInbox](https://github.com/umbertotechnopreneur/AgentInbox) and
[PromptMeUp](https://github.com/umbertotechnopreneur/PromptMeUp).

---

<p align="right">
  <a href="https://umbertogiacobbi.biz/vibeware/manifesto">
    <img src="docs/assets/vibeware/vibeware-logo.png" alt="VibeWare movement logo" width="120">
  </a><br>
  Participating in the <strong>VibeWare movement</strong><br>
  <em>Human intent, AI execution, and plenty of tokens</em><br>
  <a href="https://umbertogiacobbi.biz/vibeware/manifesto"><strong>MANIFESTO</strong></a>
</p>

I develop this project with AI under my direction. Its creator and publisher is
Umberto Giacobbi, at umbertogiacobbi.biz. Participation in VibeWare describes my
approach; it does not establish that a feature was reviewed or tested. See the
recorded [validation evidence](docs/VALIDATION.md).
