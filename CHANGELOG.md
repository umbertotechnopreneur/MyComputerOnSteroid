# Changelog

## Unreleased

### Added

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

- Visible node/shortcut name: **My Computer Enhanced**, always English.
  The project/repository remains MyComputerOnSteroid.
- Wallpaper/acrylic, its photo control and export/import are outside requested scope.

### Fixed

- Keep the Hide command decorated when only the older Segoe MDL2 font is present.
- Recent-item presentation across Shell views: preserve document names and icons,
  define native Content/Tile layouts, and prevent target property factories from
  overriding launcher metadata with serialized property-store text.
- Make all five Details columns visible by default using the Shell proplist flags.

0.0.0 installer/resource versions are development placeholders. No release/tag
or signed binary distribution has been published.
