# Completed work archive

## 2026-10-18 - Native configuration, dialogs and application artwork

The implementation was handed off locally on 2026-10-17 and is now prepared for
source delivery. These records do not close MCS-091/MCS-094 runtime acceptance.

### MCS-093 - One configuration window and discoverable setup guide

- Replaced the background settings list with Configure My Computer Enhanced.
  Added a classic namespace-icon verb and Start configuration shortcut.
- Embedded five native pages in one tabbed host, including direct favorite
  actions and About. Shared layouts, icons, localization and DPI scopes remain.
- Kept preferences in the normal user process; This PC mutations use a validated
  elevated worker with HRESULT completion and cancellation preserving the editor.
- Installed Readme Setup.txt and its Files favorite for the installing user.
- x64 Release / ARM64 Debug compilation and string parity checks passed; native
  suite/UI acceptance was not run. MSI validation retained its intentional ICE91
  per-user-shortcut warning. Final authorized repair returned 0 (the first repair
  returned 3010); installed EXE/DLL/guide hashes match. A reserved MSI Directory
  identifier collision was corrected and the misplaced guide reference retained
  in artifacts. Explorer was not restarted. Acceptance remains in MCS-094.

### MCS-092 - Compact native layout and size-specific application artwork

- Replaced tall edits and oversized action rows with native 9-point / 14-DLU
  metrics; aligned field labels on the left, right-justified to a common edge.
- Fit each localized header to its actual text height before capturing resize
  anchors. Compacted About and made both links transparent on the dialog background.
- Preserved accepted 16/24/32 PNG frames byte-for-byte, added detailed 48-256 ICO
  frames and a separate 512-pixel PNG from the generated high-resolution master.
- x64 Release / ARM64 Debug compilation passed. Visual acceptance of this
  iteration remains in MCS-091; no new native UI session or test suite was run.
  The x64 MSI passed ICE validation and the updated installation returned 3010;
  both installed hashes match Release. Explorer was not restarted.

### MCS-090 - Native dialog presentation and localization

- Shared themed PMv2 dialog presentation, source-monitor placement through
  elevation, description headers, consistent field layout and resize anchors.
- Explicit Add new / Add / Save changes flow; large icon list and preview.
- Simple About with fixed website/repository links and eight OS-selected
  application UI languages, including Hong Kong Cantonese in Traditional script.
- Pixel-art ICO with distinct small artwork, embedded in app windows and MSI.
- Final x64 Release and ARM64 Debug compilation passed; x64 MSI passed WiX ICE
  validation. The owner-authorized x64 installation returned 3010 and installed
  hashes match Release. Provider reload and visual acceptance remain open in
  MCS-091. No native suite/new UI session was run.

### MCS-080 - Readability and current-format cleanup

- Consolidated native helpers and background-menu metadata; expanded dialog/drop
  flow and added comments for ownership, validation and asynchronous launch.
- Removed the old importer, its staged migration script and the older font
  fallback. The function belongs entirely to the app's current format.
- Examples and remaining candidates are in [the review](../../docs/CODE_REVIEW.md).
- Local verification is recorded in [validation](../../docs/VALIDATION.md).
- Later local installation is recorded above; actual Explorer acceptance remains pending.


## 2026-10-12 - MCS-090: group presentation and document export

- Kept stable group IDs while putting Drives before Folders and Apps, and Office,
  PDF and Development before other families and unassigned extensions.
- Kept app favorites in one section in both dynamic grouping modes.
- Added native IDataObject adaptation for real document filenames/paths, with
  owned Shell reference identity, copy preference and no source move capability.
- x64 native tests and ARM64 compilation passed; the tested x64 MSI was installed
  and actual Explorer group/data-object observations are in validation.
- Actual GPT/desktop drag acceptance remains open; InfoTips are a proposal only.

Delivered implementation records belong here. Keep remaining verification in
[todo.md](todo.md), with detailed evidence in [validation](../../docs/VALIDATION.md).

## 2026-10-11 - MCS-080: searchable collection and extension families

- Added native section/extension/editable-family grouping, name/date ordering in
  either direction, and optional extension/family/item-date columns.
- Added name/target-path/exact-extension search over launcher references and
  enabled recents, with safe clear/no-match behavior and atomic user preferences.
- Polished the Win32 options dialog with right-aligned labels, group boxes,
  shorter date text and colored Windows stock icons.
- Passed final isolated x64 Debug/Release tests, both architecture builds and
  both native MSI ICE checks. Installed and visually checked the x64 payload in
  Explorer, including custom family rendering, search and active-view ordering.
- Restored prior private preferences after UI experiments. No target or history
  mutation was needed. Broader themes/DPI/accessibility, compact-menu deletion,
  clean installer lifecycle and ARM64 runtime remain open in MCS-060/070.

## 2026-10-11 - MCS-001: repository and native provider bootstrap

- Published the public MyComputerOnSteroid repository under Umberto Giacobbi's
  GitHub account. GitHub recognizes the canonical MIT license.
- Established Umberto Giacobbi / umbertogiacobbi.biz as creator and publisher,
  with separate VibeWare movement participation, logo provenance and AI disclosure.
- Added agent guidance, contribution/governance documents, source headers,
  issue/PR templates and todo/done/archive/lessons records.
- Added the C++20 COM factory, empty Shell provider, stable root CLSID, native
  resource identity and default Shell view construction.
- Local Debug x64 and ARM64 builds passed. The same source passed both GitHub
  architecture jobs in [run 38127650216](https://github.com/umbertotechnopreneur/MyComputerOnSteroid/actions/runs/38127650216).
- COM runtime, registration, native Explorer UI, menu and favorite behavior were
  not tested or installed. Those remain open work, not completed launcher features.

Source delivery: `ffa56301b1dc61ce1d38ff58838a7034eeb0f437`.

## 2026-10-11 - MCS-010/030/040: installed native launcher prototype

- Implemented drives and owned folder/app/file references in Explorer's DefView,
  grouping, properties, view defaults, native menus, add/drop, rename and safe
  reference policies. The visible node is My Computer Enhanced.
- Implemented launcher-only ICO changes and optional recent documents with private
  cache, typed history dates, filters and bulk pin/unpin/hide/promotion.
- Corrected recent names/icons in native views and default Details columns.
- Added native Fluent-style glyphs for custom context-menu commands. Actual recent
  commands were visible after Windows 11's Show more options, with icons in dark mode.
- Added isolated native policy/COM/menu tests, x64 CI testing, Debug/Release x64
  and ARM64 builds, native MSI authoring and guarded developer registration.
- Installed and opened the x64 development build in actual Windows 11 Explorer.
  Verified requested view modes and five Details columns after reopening.

This records local prototype delivery, not release acceptance. Remote source
delivery is tracked by Git/PR history. See [validation](../../docs/VALIDATION.md)
and [todo](todo.md) for outstanding UI, installer and ARM64 evidence.
