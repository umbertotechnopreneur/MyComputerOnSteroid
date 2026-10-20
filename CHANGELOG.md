# Changelog

## Unreleased

### Added

- Repository workspace launcher at `docs/open-code-here.bat`, which opens the
  repository root in VS Code and reports when its command is unavailable.
- Explicit targeted Windows Defender first-import policy for third-party
  prebuilt binaries and compiled libraries in the README.

- One **Configure My Computer Enhanced** command and a native tabbed window for
  This PC folders, favorites, search/view, recent items and About. It is also
  reachable directly from the This PC namespace icon and a Start shortcut.
- Installed **Readme Setup.txt**, linked from the launcher's Files section for
  the installing user and from the configuration window.

- Simple About with my website and the project GitHub link, available in the
  configuration window and from the opener's `--about` option.
- Windows display-language resources for English, Italian, Vietnamese, French,
  German, Spanish, Simplified Chinese and Hong Kong Cantonese; English fallback.
- Original application icon with pixel art for small sizes and detailed large
  artwork, embedded in the launcher, Shell DLL, dialogs, Start shortcut and MSI.

- Native **This PC folders** configuration page adds, names, icons and removes
  direct This PC folder entries without deleting real folders. It manages the
  app's current entries; only registration mutations request elevation.
- Drop one or more folders onto an empty launcher area to add them directly to
  This PC. Files and applications still become normal launcher favorites.
- Native Search and organize dialog with right-aligned labels, grouped controls
  and Windows stock icons; name/path/exact-extension search across references.
- Extension and editable-family Shell groups, name/date ordering in either
  direction, optional metadata columns and atomic per-user preferences.
- Native C++20 COM launcher in Explorer's standard view: Folders, Devices and
  drives, Apps and Files; native selection, view modes and normal sorting.
- Owned shortcut favorites, safe add/drop, argument-aware deduplication,
  reference-only delete/rename, and native context-menu delegation.
- Launcher-only ICO override with a native picker and dedicated last-folder state.
- Fluent-style icons on launcher context-menu commands, using native menu bitmaps,
  system icon fonts, DPI sizing and light/dark/high-contrast color selection.
- Optional Recent items group with name/category/extension filters, typed dates,
  bulk pin/unpin/hide, restore-hidden and promotion to favorites. Pins survive aging.
- Win32 recent-filter dialog. Concurrent refresh uses cached data; source failure
  cannot blank drives and favorites. Virtual history targets are not bound.
- Native x64/ARM64 Release MSI, per-machine namespace registration, Start-menu
  opener and separate guarded developer registration.
- Debug/Release presets, isolated policy/COM/menu tests and x64 CI tests.
- MIT licensing, creator identity, VibeWare participation, provenance and task records.

### Changed

- Replaced the scattered background settings commands with a single Configure
  entry. Settings pages share one DPI-aware window; favorite actions are built in.
- Kept per-user configuration unelevated and isolated This PC registry writes in
  a checked worker, preserving the editor on UAC cancellation.

- Tightened every dialog with standard-height single-line edits, right-aligned
  labels to their left, smaller action buttons and content-sized header text.
  About now uses a compact application icon/title header and transparent links.
- Kept the accepted 16/24/32-pixel icons unchanged; supplied detailed artwork
  for 48-256 pixels and a separate high-definition 512-pixel PNG.

- Reworked every application dialog with a description in its header, themed
  native controls, consistent spacing and resizable layouts.
- The This PC folder manager now has explicit Add new / Add / Save changes modes,
  a 96-logical-pixel icon list, icon preview and selection-dependent actions.

- Shortened the README and cleanup notes with plain steps and small examples.
- Consolidated process launching, Windows argument quoting, module lookup and
  dialog-text reads; documented asynchronous results and resource ownership.
- Replaced numeric background-menu dispatch and nested glyph selection with a
  single command table and named actions. Shared the default/Apply sort columns.
- Separated view-dialog setup from validation, expanded dense dialog/drop code,
  and named bitmap color channels and other intermediate results.
- Removed the old importer and older MDL2 font fallback. The application handles
  only current registrations; no migration scripts are retained.
- Made the README shorter and clearer: it now explains why I built the
  launcher, that Explorer keeps its normal view, and how to group items by
  section and order them by name.
- Visible node/shortcut name: **My Computer Enhanced**, always English.
  The project/repository remains MyComputerOnSteroid.
- Wallpaper/acrylic, its photo control and export/import are outside requested scope.

### Fixed

- Declare PerMonitorV2 for the opener and scope Shell UI DPI/visual styles to its
  calling thread. Let Windows rescale dialog fonts and use DPI-aware layout/icons.
- Place dialogs on the invoking window's monitor, or the pointer's monitor when
  there is no window owner. Preserve the source rectangle across UAC relaunches.

- Put Devices and drives before Folders and Apps; keep app favorites in one group.
  Family groups prioritize Office, PDF and Development, with unassigned suffixes last.
- Export original file paths for favorite/recent document drag and drop while
  retaining owned shortcut identities for native Shell operations; prefer copy.
- Keep the Hide command decorated when only the older Segoe MDL2 font is present.
- Recent-item presentation across Shell views: preserve document names and icons,
  define native Content/Tile layouts, and prevent target property factories from
  overriding launcher metadata with serialized property-store text.
- Make all five Details columns visible by default using the Shell proplist flags.

0.0.0 installer/resource versions are development placeholders. No release/tag
or signed binary distribution has been published.
