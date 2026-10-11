# Architecture and implementation status

## Current source

The repository contains a C++20 in-process COM DLL. Its class factory creates an
empty `IShellFolder2` / `IPersistFolder2` provider. `CreateViewObject` calls
`SHCreateShellFolderView` to construct the system's default folder-view object
(DefView). This is a source scaffold; an actual Explorer-hosted view has not been
registered, displayed or verified.

| Component | Responsibility | Current state |
| --- | --- | --- |
| `src/ProjectIdentity.hpp` | Creator, publisher, URLs and launcher CLSID | Implemented |
| `src/core/Section.hpp` | Stable section identifiers and favorite-section policy | Implemented contract |
| `src/shell/Dll.cpp` | COM factory and DLL lifetime accounting | Implemented, compile-verified |
| `src/shell/ShellFolder.*` | Empty enumeration, root persistence, Name column and DefView construction | Implemented, compile-verified |
| `resources/` | Windows version identity and UI resource strings | Implemented |
| `scripts/Build.ps1` | Installed toolchain discovery and isolated Debug builds | Implemented |

The DLL exports `DllGetClassObject` and `DllCanUnloadNow`. It has no self-registration
exports, installer, drop target, selected-item context menu or favorite storage.
Unsupported operations return failure; they do not simulate completed work.

## Intended integration contract

Explorer opens one launcher namespace root, identified by CLSID
`{B36B9212-2D35-4E65-8A97-0AA5D280E157}`. Favorite folders, apps and files will not
each require a registered CLSID. The proposed data model separates system drives
from local favorite references.

One standard Shell view should present four groups in this order: Folders,
Devices and drives, Apps, Files. A category provider or section property supplies
group identity. View modes and normal sorting are delegated to the Shell;
manual item ordering is outside the current contract. Empty groups and grouping
in list mode need actual-host verification.

The adapter will own PIDL translation and delegate appropriate operations to
the real Shell items. Favorite storage and mutation rules belong beneath that
adapter. App shortcuts may represent executables or packaged Shell items;
classification must not execute a target or assume every app is an EXE.

## Mutation contract

- A drop adds a reference. It must not move or copy the original file/folder.
- Removing a favorite removes only its launcher reference. Do not delegate a
  destructive verb to the target's menu and assume it will remain safe.
- Drives cannot be removed from the launcher. Ordinary drive context-menu
  operations are separate from removing a favorite.
- Shortcut resolution, duplicate handling and unavailable targets need explicit
  behavior. Writes must not leave partial favorite records after failure.
- An icon override belongs to the launcher item only. Use the native file dialog
  and persist its last icon-selection location, with a valid fallback if gone.

These rules are requirements, not evidence of implementation in the empty provider.

## Context-menu gate

The first integration experiment must run in actual File Explorer. A standalone
`IExplorerBrowser` host or a hand-built ListView is a different environment.
Record background, single-item and mixed-selection results separately, for
physical shortcuts, virtual items and drives.

`IContextMenu` offers the native classic menu contract. Its presence does not
establish Windows 11 compact-menu rendering. `IExplorerCommand` supplies commands
to Explorer and is not a public function that displays the whole modern popup on
an arbitrary window. Preserve **Show more options** when Explorer supplies it.

## Background gate

No public API was established for applying arbitrary wallpaper or acrylic to a
normal DefView. `FWF_DESKTOP` and `FWF_TRANSPARENT` are documented for the desktop.
DWM backdrop settings alone do not make an opaque child view transparent.

The requested lower-right photo icon and background may require a custom native
`IShellView` surface. That choice changes the context-menu integration problem.
Do not implement private Explorer hooks or promise all three requirements without
evidence. A later decision must explicitly balance Shell fidelity and background
control; both implementation paths can remain native C++.

## Public references

- [Default Shell view](https://learn.microsoft.com/en-us/windows/win32/api/shlobj_core/nf-shlobj_core-shcreateshellfolderview)
- [Microsoft C++ Explorer Data Provider sample](https://learn.microsoft.com/en-us/samples/microsoft/windows-classic-samples/explorerdataprovider/)
- [Category providers](https://learn.microsoft.com/en-us/windows/win32/api/shobjidl_core/nn-shobjidl_core-icategoryprovider)
- [View modes and icon sizes](https://learn.microsoft.com/en-us/windows/win32/api/shobjidl_core/nf-shobjidl_core-ifolderview2-setviewmodeandiconsize)
- [Windows 11 context-menu commands](https://learn.microsoft.com/en-us/windows/apps/desktop/modernize/integrate-packaged-app-with-file-explorer)
- [Folder flags and desktop-only transparency](https://learn.microsoft.com/en-us/windows/win32/api/shobjidl_core/ne-shobjidl_core-folderflags)
- [DWM system backdrops](https://learn.microsoft.com/en-us/windows/win32/api/dwmapi/ne-dwmapi-dwm_systembackdrop_type)

Documentation establishes the public contracts. It does not replace runtime
verification of this provider on the Windows build being used.
