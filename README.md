# MyComputerOnSteroid

**A native launcher inside File Explorer for your drives, favorite folders, apps and files.**

I keep returning to the same folders and documents between projects. This
C++20/Win32/COM provider puts their shortcuts beside my drives in Windows'
standard Shell view. Explorer draws the view and supplies its normal selection,
icons, list/details modes and sorting.

The visible node is **My Computer Enhanced**, always in English. The project
and repository keep the name MyComputerOnSteroid.

**Current status: an installed x64 development prototype.** Native policy/COM
tests and actual Explorer observations are recorded in [validation](docs/VALIDATION.md).
ARM64 compilation and MSI authoring do not establish ARM64 runtime behavior.
The 0.0.0 MSI is a development package; no application release is published.

[![Native build](https://github.com/umbertotechnopreneur/MyComputerOnSteroid/actions/workflows/build.yml/badge.svg)](https://github.com/umbertotechnopreneur/MyComputerOnSteroid/actions/workflows/build.yml)
[![MIT](https://img.shields.io/badge/license-MIT-4D91A8)](LICENSE)

## Launcher

- **Folders**, **Devices and drives**, **Apps**, **Files**, and optional **Recent items**.
- Native icon, list and details modes, ordinary sorting and Ctrl/Shift selection.
- Drop-to-add references, target/argument-aware deduplication, favorite rename
  and reference-only removal. Original items stay in place; drives cannot be
  removed as favorites.
- Explorer's context-menu integration. Its Windows 11 compact menu, including
  **Show more options**, was observed in the x64 host. Additional launcher
  commands use the classic menu contract, with native Fluent-style glyphs.
  Host-specific checks remain explicit.
- **Change launcher icon**: choose an ICO in a native file dialog. Only the owned
  favorite link changes. Windows keeps a dedicated last-folder state for this
  picker. The selected icon file is referenced at its existing path.
- Optional recent documents with filters, a typed date column, bulk pin/unpin,
  hide, restore-hidden and promotion to favorites.

Right-click empty space and use Show more options for Add favorite folder/app/file,
Manage favorite shortcuts, Show recent items, Filter recent items and Restore
hidden recent items. Select favorite references for Change launcher icon.
Recent-item commands act on the complete selected set.

## Recent items

Recent items are initially off. The native filter dialog enables the fifth group
and filters only that group by name, category and optional exact extension.
Categories are Documents (PDF, Word, Excel, Markdown...), Images, Audio, Video,
Development (CSS, HTML, JavaScript, C++...) and Other. Enter pdf, .pdf, jpg or md
in the extension field, or leave it empty. The group uses normal Shell collapsing.

The source is Windows' **Recent Items** known folder, populated by the Shell and
participating applications. It is not a complete access audit or a merge of every
application's Jump List. **Recent entry updated** means the Windows shortcut's
update time, not the document's modification time or a guaranteed last-opened time.
At most the latest 512 input shortcuts are considered per refresh; only filesystem
targets are indexed. Use Refresh to pick up new Windows history.

Pin keeps a private reference when Windows ages out the source entry. Hide also
unpins. Delete on recent items means hide locally. Ctrl/Shift selection allows
bulk pin, unpin, hide and Add to launcher favorites. Filters apply to pins too.
Documents and Windows history are preserved by these operations.

Favorites and private recent state live under
%LOCALAPPDATA%\MyComputerOnSteroid\. They are not uploaded or included in the MSI.
Wallpaper/acrylic, its photo control and export/import are outside the current
owner-requested scope. There is no managed or web UI runtime.

## Build, test and package

Use Windows 11, PowerShell 7, Visual Studio with Desktop development with C++,
Windows SDK and CMake/Ninja. Add ARM64 tools for that target and WiX 3.14 for MSI.

~~~powershell
pwsh -NoProfile -File ./scripts/Build.ps1 -Architecture x64 -Configuration Release
pwsh -NoProfile -File ./scripts/Test.ps1 -Architecture x64 -Configuration Release -SkipBuild
pwsh -NoProfile -File ./scripts/Build-Msi.ps1 -Architecture x64 -SkipBuild
~~~

Output is under ignored artifacts/. Building does not register, install or restart
Explorer. Install the matching native-architecture MSI to add My Computer Enhanced
under This PC and the navigation tree. Launch its Start-menu shortcut to open it.
See [build/install instructions](docs/BUILD.md) and [architecture](docs/ARCHITECTURE.md).

## Development

- [Contributing](CONTRIBUTING.md) and [agent instructions](AGENTS.md).
- [Open work](.github/tasks/todo.md), [completed work](.github/tasks/archive.md),
  [changelog](CHANGELOG.md) and [engineering lessons](.github/tasks/lessons.md).
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
Umberto Giacobbi, at umbertogiacobbi.biz. VibeWare describes participation in a
movement, not a manufacturer or a feature's review/testing status.
