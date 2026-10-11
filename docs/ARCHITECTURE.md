# Native Shell launcher

The C++20 COM DLL supplies one IShellFolder2 namespace root to Windows' default
Shell view (DefView). Explorer draws the items, groups, selection, view modes and
normal sorting. The visible name is **My Computer Enhanced**.

## Components

| Component | Responsibility |
| --- | --- |
| core/Catalog | Drives; owned favorites; classification, duplicate, rename, icon and removal policy |
| core/Recents | Optional Windows history source, private cache, filters, dates, pins and hiding |
| native/Support | COM/PIDL ownership, HRESULT boundaries, resource strings and diagnostics |
| native/RecentDialog | Standard Win32 filter dialog: edit, combo, checkbox and buttons |
| native/MenuIcons | DPI-sized, theme-aware glyph bitmaps owned for native menu lifetime |
| shell/ItemId | Validated child PIDLs and delegation to real Shell items |
| shell/Grouping | Section category provider |
| shell/FavoriteMenu | Native menu delegation, reference-only mutations and added commands |
| shell/ShellFolder | Enumeration, properties, DefView, grouping, menus and drop |
| shell/Dll | Class factory and DLL lifetime |
| Launcher | Native namespace opener and optional add-favorite command line |
| installer/Product.wxs | Native-architecture per-machine MSI and namespace registration |

The DLL exports DllGetClassObject and DllCanUnloadNow, not self-registration
exports. COM entry points convert exceptions to HRESULTs. Interfaces, PIDLs and
allocated strings have explicit ownership. DllMain performs no I/O.

## Identity and view

CLSID: {B36B9212-2D35-4E65-8A97-0AA5D280E157}. Canonical root:

~~~text
::{20D04FE0-3AEA-1069-A2D8-08002B30309D}\::{B36B9212-2D35-4E65-8A97-0AA5D280E157}
~~~

Individual favorites do not need CLSIDs. Stable sections are Folders 1, Drives 2,
Apps 3, Files 4, Recent items 5. A category provider supplies groups; empty groups
are omitted. Shell group collapsing, view modes and sorting remain native.
No manual item ordering is implemented.

Columns are Name, Section, Location, Recent entry updated and Pinned in recent
items. The date is typed for native sorting; unrelated items have no recent date.
Default sorting requests date descending and name ascending. Saved Explorer
preferences can override defaults. The date maps to the Shell date-modified
property for the **history entry**, not the target document.

The column proplist uses the documented 0 flag for each visible default column;
an unflagged property is included by the property system but hidden in DefView.

Custom commands retain native HMENU text/interaction and receive MIIM_BITMAP
glyphs. Segoe Fluent Icons comes from the installed OS, with Segoe MDL2 Assets
as a compatibility fallback; no font is redistributed. The public
[Microsoft glyph table](https://learn.microsoft.com/en-us/windows/apps/design/iconography/segoe-fluent-icons-font)
defines the selected star, pin, unpin, hide, photo, folder, app, document, filter,
history and restore symbols. GDI creates premultiplied-alpha bitmaps at the menu
owner's DPI. Colors follow the Windows app light/dark preference or system menu
text in high contrast. The menu object retains bitmap ownership, including older
DPI/theme variants. Missing fonts or decoration failure leave the text commands
available. No owner-drawn replacement or private Explorer theming API is used.
Font versions can differ even within a family. If Hide cannot render, try the
compatible Blocked symbol in either installed icon font.

References supply their name aliases, content/tile property lists and the
documented content layout through GetDetailsEx. Launcher items decline target
property-store binding so DefView uses the launcher properties. Returning a target's property
factory bypassed these overrides and produced hexadecimal property-store text in
Content/small-icon modes on the tested host. Other unavailable display properties
remain empty; native target properties keep their typed values.

## Owned favorites

Favorite LNK files live in Folders, Apps and Files under
%LOCALAPPDATA%\MyComputerOnSteroid\Favorites\. Adding an existing shortcut
preserves its target PIDL, arguments, working directory and icon. Deduplication
uses target identity plus launch arguments within the selected section.
Classification does not execute the target or call IShellLink::Resolve.

Writes use temporary links and atomic rename. Delete, rename and icon changes
validate the exact owned directory and reject reparse points. A favorite's target
is never substituted for its shortcut during mutation. System drives are separate,
non-removable catalog items; their ordinary drive menu remains delegated to Windows.

The icon picker accepts a valid ICO and changes only the favorite's icon location.
IFileDialog::SetClientGuid gives it dedicated Windows-maintained last-folder state.
The icon file is referenced in place, not copied into the application or target.

## Optional recent documents

Recent items are off by default. The source is FOLDERID_Recent, read directly as
Windows-managed shortcuts. The latest 512 inputs are considered. Malformed and
virtual namespace entries are skipped. Filesystem paths are read from links
without binding virtual providers, repairing links, walking target directories
or inspecting document contents.

History reflects participating applications and Shell operations. It is not an
access audit, exact last-opened timestamp, universal frequency list or union of
all Jump Lists. The date is **LastWriteTime of the Windows history shortcut**.

Private cache links and UTF-16 metadata live under Recents/Items/. A SHA-256 key
identifies normalized target path plus arguments. Metadata records display name,
source date, extension, source presence, pin and hidden state. Windows history
and its shortcuts are never changed. Pins survive source aging; hide also unpins.
Restore clears local hide state. Filters apply to pinned entries too.

Case-insensitive name substring, category and exact extension filters combine.
The native filter dialog configures only the fifth group; DefView draws all groups.
Ctrl/Shift selection allows bulk pin, unpin, hide and promotion to favorites.
Delete means hide for recent items. Direct Shell deletion of a cache link is
recognized as a local hide on refresh, preventing silent resurrection.

A per-store mutex coordinates refresh and mutations across Explorer windows.
Enumeration uses cached entries if a refresh already owns the mutex. Mutations
release the mutex before Shell notification, which can re-enter enumeration.
Cache sorting reads metadata once per entry. Failure of the optional source is
logged and cannot suppress drives or saved favorites.

Refresh reads new history. No polling thread or watcher is installed.
MCOS_DATA_DIR and MCOS_RECENT_DIR provide isolated test inputs; the latter requires
an explicit data-store override. Normal Explorer sessions use known folders.

## Menus and drop

Real Shell menus provide ordinary operations. An IContextMenu3/IObjectWithSite
adapter adds the icon and recent-item commands. It handles numeric, ANSI and
Unicode verbs and intercepts Delete below the UI. It retains its COM identity
rather than forwarding arbitrary interfaces that could bypass mutation policy.

Background commands merge through IContextMenuCB. DFM_INVOKECOMMAND and
DFM_INVOKECOMMANDEX both dispatch add/manage/show/filter/restore actions.
Added commands use the classic contract. IContextMenu alone does not establish
a Windows 11 popup. Actual host/build observations are in [validation](VALIDATION.md).

Drop binds a valid Shell item array/data object, including selections from
different parents. It negotiates link, or copy if link is unavailable, never move.
Only references are created. Partial failures preserve references already added.

## Installation and boundaries

The MSI installs Release binaries under Program Files, registers one root under
This PC and adds a Start-menu opener. Windows 11 and the matching native x64/ARM64
architecture are required. Per-user favorites and recent state are not removed
on uninstall. Developer per-user registration is separate and refuses registration
over a machine MSI installation.

Wallpaper/acrylic and export/import were removed from scope. No custom surface,
private Explorer hooks, injected controls, managed UI or web runtime is used.
The native filter dialog preserves the accepted DefView architecture.

## Public contracts

- [Default Shell view](https://learn.microsoft.com/en-us/windows/win32/api/shlobj_core/nf-shlobj_core-shcreateshellfolderview)
- [Category providers](https://learn.microsoft.com/en-us/windows/win32/api/shobjidl_core/nn-shobjidl_core-icategoryprovider)
- [View modes](https://learn.microsoft.com/en-us/windows/win32/api/shobjidl_core/nf-shobjidl_core-ifolderview2-setviewmodeandiconsize)
- [Recent document source](https://learn.microsoft.com/en-us/windows/win32/api/shlobj_core/nf-shlobj_core-shaddtorecentdocs)
- [File-dialog persisted state](https://learn.microsoft.com/en-us/windows/win32/api/shobjidl_core/nf-shobjidl_core-ifiledialog-setclientguid)
- [Extended menu invocation](https://learn.microsoft.com/en-us/windows/win32/shell/dfm-invokecommandex)
- [Data object child-PIDL requirements](https://learn.microsoft.com/en-us/windows/win32/api/shlobj_core/nf-shlobj_core-shcreatedataobject)
