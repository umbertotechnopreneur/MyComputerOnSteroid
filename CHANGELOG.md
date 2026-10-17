# Changelog

## Unreleased

### Added

- Native **Manage This PC folders** command in the launcher background menu.
  Its elevated Win32 dialog adds, names, icons and removes direct This PC folder
  entries without deleting real folders. It manages the app's current entries.
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
