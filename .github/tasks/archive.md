# Completed work archive

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
