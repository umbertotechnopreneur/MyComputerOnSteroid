# Native Shell launcher

The C++20 COM DLL supplies one IShellFolder2 namespace root to Windows' default
Shell view (DefView). Explorer draws the items, groups, selection, view modes and
normal sorting. The visible name is **My Computer Enhanced**.

## Components

| Component | Responsibility |
| --- | --- |
| core/Catalog | Drives; owned favorites; classification, duplicate, rename, icon and removal policy |
| core/Recents | Optional Windows history source, private cache, filters, dates, pins and hiding |
| core/ViewSettings | Atomic preferences, target metadata, search and extension-family policy |
| native/Support | COM/PIDL ownership, HRESULT boundaries, process/argument helpers, dialog text, resources and diagnostics |
| native/Dialog | Scoped visual styles and DPI, shared localized dialog layout, monitor placement and About |
| native/Configuration | Single tabbed settings host, favorite actions and public Shell view refresh |
| native/RecentDialog | Standard Win32 filter dialog: edit, combo, checkbox and buttons |
| native/ViewDialog | Standard Win32 search/group/order dialog and extension-family editing |
| native/MenuIcons | DPI-sized, theme-aware glyph bitmaps owned for native menu lifetime |
| native/ThisPcEntries | This PC settings page and validated elevated registration worker |
| shell/ItemId | Validated child PIDLs and delegation to real Shell items |
| shell/TransferData | Reference Shell identity with document target file-transfer formats |
| shell/Grouping | Section, extension and editable family category providers |
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

Category comparison uses presentation ranks instead of numeric section IDs:
Drives, Folders, Apps, Files, Recent items. In family mode Office, PDF and
Development/Sviluppo precede other naturally sorted families. Other extensions
and files without an extension are last. Apps stay one section in both dynamic
grouping modes. Group IDs retain their previous meanings. Item name/date order
does not change these ranks; Explorer's descending group choice can reverse them.

The original visible columns are Name, Section, Location, Recent entry updated
and Pinned in recent items. Extension, Extension family and Item date are optional
columns, mapped to System.FileExtension, System.KindText and System.ItemDate.
Item date is a typed target modification date for local favorites or history
entry update time for recent items. Network targets, missing dates and drives
return empty dates. The original date-modified mapping remains the history entry
only. Date ordering adds name ascending as its tie-breaker. Explorer's saved view
preferences can override provider defaults.

The column proplist uses the documented 0 flag for each visible default column;
an unflagged property is included by the property system but hidden in DefView.

## Collection search and organization

Enumeration applies ViewSettings after the existing recent policy. Empty queries
match all items. Name matches the launcher alias; Path matches the link target;
combined mode matches either, including the target suffix. Matching is a
case-insensitive substring except Extension, which is exact and normalizes an
optional dot. No directory traversal, content search or Windows Search index is
involved. Explorer's own search box is not replaced.

IShellFolder2/IFolderViewSettings expose the new properties and ordering defaults.
ICategoryProvider supplies separate categorizer identities for sections, target
extensions and editable families. Drives and folders keep their section groups;
other items use target suffixes rather than .lnk or renamed aliases. Dynamic
category names at the same presentation rank use StrCmpLogicalW. A categorizer reads current family
definitions per batch so edits do not leave stale classification.

The single background command launches the native opener with `--configure` and
the source window rectangle. A localized classic verb on the namespace CLSID and
a Start shortcut reach the same window without first entering the launcher.
After Apply, the opener finds launcher views through public IShellWindows,
IShellBrowser and IFolderView services, verifies their exact root PIDLs and uses
IFolderView2 SetGroupBy/SetSortColumns/SetText and Refresh. Preferences apply to
every discovered active launcher view, with a namespace notification as well.
COM view references are scoped to the refresh; no private HWND assumptions,
Explorer process changes or persistent folder/view cycles are introduced.

The per-user view.ini is a bounded UTF-16 preference record, not a conventional
INI. It holds enum choices, query and multiline family definitions. Names and
extension assignments are unique ignoring case. Writes validate first, use a
unique CREATE_NEW temporary file, flush, and atomically replace the complete
record. Concurrent Apply uses the last complete writer. Cancel never writes;
corrupt/unreadable optional preferences fall back to unfiltered sections.
Owned paths reject reparse points. Link metadata is read without Resolve or target
execution; optional date lookup skips network targets.

The dialogs use standard edit, combo, list-view and button controls. Each dialog
has a description in its header and a common action row. A 9-point Segoe UI font,
14-dialog-unit single-line edits, compact buttons and right-aligned labels to the
left of fields keep native metrics and consistent baselines. Header copy is
measured with the actual dialog font after localization; the separator, content
and window height move together before resize anchors are captured. This removes
unused header space while preserving wrapped translations and normal control sizes.
The This PC manager uses an icon list at 96 logical pixels, previews the selected
icon, and distinguishes adding a new entry from editing a selection. Icon
extraction does not execute its source file; image lists copy the pixels and own
no target folder. Handles and fonts are released after the dialog closes.

Configuration uses one modal host and a native tab control: This PC folders,
Favorites, Search and view, Recent items and About. Existing language-neutral
DIALOGEX templates are cloned as DS_CONTROL child pages, with explicit controller,
HWND and presentation-frame lifetimes. Child pages share the host's message loop,
font/DPI scope and keyboard navigation; hidden modal footer buttons cannot act.
Preference pages validate before writes. Apply saves both preference groups and
then refreshes discovered launcher views. The two preference files are not one
transaction: an I/O/refresh failure can leave earlier saves applied and is reported
as such. Close does not save pending preference edits. This PC/favorite actions
and restoring hidden recent entries are immediate, separate operations.

The configuration process stays unelevated. This PC mutations use a hidden
elevated opener worker, not another settings window. The worker checks the
original user's SID, GUID syntax, target directory and ownership in both hives;
different-account elevation is rejected rather than writing another user's HKCU.
The owner window is disabled while a message-pumping wait obtains the worker's
HRESULT exit code. Cancelling UAC preserves the editor and makes no mutation.

One language-neutral dialog template per window keeps layout identical across
translations. String blocks are selected explicitly from Windows display-language
preferences without changing Explorer's thread language. Supported resources are
en-US, it-IT, vi-VN, fr-FR, de-DE, es-ES, zh-CN and zh-HK; English is the fallback.
Hong Kong/Macau Chinese and yue preferences select the Cantonese resources in
Traditional characters; other Chinese preferences select Simplified Chinese.
Existing user-defined family names are data and are not translated on load.

The opener manifest declares PerMonitorV2 and Common Controls v6. The DLL embeds
its own v6 dependency and uses the SDK's isolation-aware controls. A UI scope
activates the current module's manifest and restores the previous thread DPI and
activation contexts on exit; it never changes Explorer's process DPI mode.
Windows handles PMv2 font/control scaling. Shared anchors handle user resizing
from original DPI-normalized bounds; application icons are reloaded at the new
DPI. Dialog descriptions and backgrounds use system colors for high contrast.

Dialog placement uses the source top-level window rectangle and clamps to that
monitor's work area. With no usable owner it uses the pointer's monitor. The
configuration command line carries a bounded source rectangle, not a foreign HWND.
It does not persist placement. A hidden positioning owner supplies the initial
monitor/DPI to the out-of-process configuration window. Elevated registration
workers display no settings window of their own.
About uses a compact icon/title header and transparent keyboard-accessible SysLink
controls for two fixed URLs. Its body avoids repeating the localized window title.
Error/validation messages use standard TaskDialog UI with the application icon.
File-type icons and native common file/folder pickers keep Windows associations.

The application icon keeps the accepted 16/24/32-pixel artwork unchanged. Pixel
art is used below 48 pixels; a separate detailed master supplies the 48-256 pixel
frames. Its ICO contains ten sizes from 16 to 256 pixels. A separate 512-pixel
PNG is prepared from the high-resolution detailed master, outside the ICO format.
Asset provenance, hashes and generation prompts are in [ASSETS.md](ASSETS.md).

## Native presentation

Custom commands retain native HMENU text/interaction and receive MIIM_BITMAP
glyphs. Segoe Fluent Icons comes from the installed Windows 11 OS; no legacy font
fallback or redistributed font is included. The public
[Microsoft glyph table](https://learn.microsoft.com/en-us/windows/apps/design/iconography/segoe-fluent-icons-font)
defines the selected star, pin, unpin, hide, photo, folder, app, document, filter,
history and restore symbols. GDI creates premultiplied-alpha bitmaps at the menu
owner's DPI. Colors follow the Windows app light/dark preference or system menu
text in high contrast. The menu object retains bitmap ownership, including older
DPI/theme variants. Missing fonts or decoration failure leave the text commands
available. No owner-drawn replacement or private Explorer theming API is used.
Font versions can differ even within a family. If Hide cannot render, try the
documented Remove symbol (E738) in either installed icon font.

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

The MSI installs Readme Setup.txt in the application directory and creates an
ordinary Files favorite shortcut for the installing user. It uses the same owned
reference/PIDL contracts as other favorites; no built-in item type or data-format
fallback is introduced. The installer owns that one shortcut, not other favorites.
Its per-user placement is intentional in the per-machine package (ICE91 warning):
the shortcut is not automatically copied into every Windows user's profile.
Removing it in the launcher preserves the installed guide; a later repair can
restore the MSI-owned shortcut. The configuration window also opens the guide.

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
One command table supplies actions, labels, glyphs and error resources; the
invocation offset selects that table entry rather than a second numeric mapping.
Added commands use the classic contract. IContextMenu alone does not establish
a Windows 11 popup. Actual host/build observations are in [validation](VALIDATION.md).

Drop binds a valid Shell item array/data object, including selections from
different parents. A complete drop of real filesystem folders opens the direct
This PC manager route; all other drops create launcher references. It negotiates
link, or copy if link is unavailable, never move. Partial reference failures
preserve references already added.

Outgoing document transfers wrap two standard Shell data objects. CF_HDROP,
FileName and descriptor/content formats come from the real filesystem document
targets for Files and Recent items. Folder/app selections retain their references
in those formats. CFSTR_SHELLIDLIST and other Shell identity formats continue to
come from the owned links, so Shell reference operations do not acquire document
identities. Each format is routed consistently in GetData, QueryGetData and
enumeration; unavailable target formats never fall back to hashed cache links.
Mixed-parent selections use IShellItemArray rather than invalid relative PIDLs.

The export advertises Preferred DropEffect COPY and clears SFGAO_CANMOVE at the
source. Destination SetData feedback is retained by the reference object, never
forwarded to the target object. The adapter exposes only its own IDataObject COM
identity and adds no document deletion. Missing/non-filesystem document targets
report failure. These are source-side policies, not control over arbitrary actions
performed by a receiving application after it obtains filesystem paths.

## Installation and boundaries

The MSI installs Release binaries under
`Program Files\Umberto Giacobbi.biz\MyComputerOnSteroid`, registers one root under
This PC and adds a Start-menu opener. Windows 11 and the matching native x64/ARM64
architecture are required. Per-user favorites and recent state are not removed
on uninstall. Developer per-user registration is separate and refuses registration
over a machine MSI installation.

Direct This PC folders are a separate Shell feature from launcher favorites. The
launcher background menu opens its own standard Win32 manager process. Dropping one
or more real folders onto an empty launcher area follows the same path and asks for
one elevation prompt for the complete drop. That process requests elevation, because
each owned folder needs matching HKCU and HKLM CLSID
and This PC namespace registrations. It writes only entries marked as owned by
MyComputerOnSteroid. Add/change/remove never touches the real target folder.
The application supports only its current MCOS registration format. The registry
is its source of truth. No importer, migration script, old JSON reader or
legacy-marker scan is included. The manager does not restart Explorer.

Wallpaper/acrylic and export/import were removed from scope. No custom surface,
private Explorer hooks, injected controls, managed UI or web runtime is used.
The native filter dialog preserves the accepted DefView architecture.

## Public contracts

- [Default Shell view](https://learn.microsoft.com/en-us/windows/win32/api/shlobj_core/nf-shlobj_core-shcreateshellfolderview)
- [Category providers](https://learn.microsoft.com/en-us/windows/win32/api/shobjidl_core/nn-shobjidl_core-icategoryprovider)
- [Apply grouping](https://learn.microsoft.com/en-us/windows/win32/api/shobjidl_core/nf-shobjidl_core-ifolderview2-setgroupby)
- [Item date property](https://learn.microsoft.com/en-us/windows/win32/properties/props-system-itemdate)
- [Windows stock icons and ownership](https://learn.microsoft.com/en-us/windows/win32/api/shellapi/nf-shellapi-shgetstockiconinfo)
- [View modes](https://learn.microsoft.com/en-us/windows/win32/api/shobjidl_core/nf-shobjidl_core-ifolderview2-setviewmodeandiconsize)
- [Recent document source](https://learn.microsoft.com/en-us/windows/win32/api/shlobj_core/nf-shlobj_core-shaddtorecentdocs)
- [File-dialog persisted state](https://learn.microsoft.com/en-us/windows/win32/api/shobjidl_core/nf-shobjidl_core-ifiledialog-setclientguid)
- [Extended menu invocation](https://learn.microsoft.com/en-us/windows/win32/shell/dfm-invokecommandex)
- [Data object child-PIDL requirements](https://learn.microsoft.com/en-us/windows/win32/api/shlobj_core/nf-shlobj_core-shcreatedataobject)
