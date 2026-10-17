# Open engineering work

Only open tasks belong here. Each task has an observable completion condition.
Move implemented work to [done.md](done.md), then to [archive.md](archive.md) after
delivery. Keep remaining validation open.

## MCS-091 - Accept the updated native dialogs

- [x] Install the owner-approved development package. Installed x64 EXE/DLL hashes
  match the Release build. Earlier repairs returned 3010 (restart required);
  the latest corrected configuration package repair returned 0.
- [ ] Reload the updated provider after an authorized Explorer/system restart
  and verify the tabbed configuration plus standalone About, keeping install and
  visual evidence separate.
- [ ] Verify 100/125/150/200/225/300% DPI, resizing and moves between monitors;
  check initial placement from each Explorer window and after UAC elevation.
- [ ] Verify Add new, selection/edit, icon preview, save/remove, Cancel and
  keyboard operation with disposable current-format entries; preserve targets.
- [ ] Inspect all eight translations for clipping, glyph coverage and wording,
  including English fallback and the Hong Kong/Macau Cantonese mapping.
- [ ] Check small title/Start/MSI icons, both About links and system high contrast.
  Compilation/resource checks do not establish visual or accessibility acceptance.
- [ ] Accept the compact 9-point / 14-DLU layout and text-sized headers in the
  current revision. Verify transparent About links and detailed large Explorer
  icons after cache/provider refresh, preserving the accepted small artwork.

## MCS-094 - Accept unified configuration

- [ ] After provider reload, verify the single localized Configure command on
  the launcher background and namespace icon, including Show more options;
  verify the Start shortcut and installed setup-guide favorite opening.
- [ ] Exercise all five tabs, keyboard traversal, resizing and mixed-DPI moves;
  retain MCS-091's eight-language/high-contrast/accessibility acceptance.
- [ ] With disposable current-format entries, verify UAC save/remove, cancellation,
  worker error propagation, GUID/ownership checks and different-account rejection.
  Verify real targets and unrelated keys survive; source checks are not runtime proof.
- [ ] Verify favorite add/remove/icon actions and immediate restore-hidden behavior.
  Check Apply/Close, invalid families/recent filters, persistence failures and
  view refresh against open launcher windows without changing unrelated views.
- [ ] On a clean install/uninstall/repair, verify the installing-user guide
  shortcut lifecycle and preservation of unrelated favorites. Keep the deliberate
  ICE91 per-user shortcut warning distinct from validation errors.

## MCS-060 - Complete Explorer interaction acceptance

- [ ] Verify document drag into GPT and other file receivers with a disposable
  file, plus desktop drop. Native transfer tests and actual Explorer IDataObject
  checks pass; receiving-application acceptance is still separate evidence.
- [ ] Resolve and verify persistent removal through Windows 11's compact Delete
  command. An earlier UI experiment removed a row but left its owned link in place.
  Require removal after reopening, target preservation and drive protection.
- [ ] Exercise classic/compact menus on background, drives, favorites and mixed
  selections; verify exact selection, keyboard use and the icon picker's last
  directory. Keep this separate from isolated COM menu tests.
- [ ] Verify recent pin/unpin/hide/promotion with actual Ctrl/Shift selection,
  date sorting, collapse and combined dialog filters. Core tests already pass;
  the owner deferred further recent-management UX work.
- [ ] Check menu icons and native views in light and high-contrast themes and
  across monitor DPI changes. Dark-theme recent-menu icons were observed locally.

## MCS-070 - Installer lifecycle and architecture acceptance

- [ ] On a clean Windows 11 x64 machine, prove install/upgrade/uninstall removes
  owned registration and binaries, preserving private user state and unrelated
  keys. The current workstation repair is not clean-machine lifecycle evidence;
  the latest check found no per-user InprocServer32 override.
- [ ] Validate ARM64 runtime on native ARM64 Windows 11. Cross-compilation and
  MSI ICE checks alone do not satisfy this task.
- [ ] Record a source/security/accessibility review before a release. The owner
  selects a version; 0.0.0 packages remain development-only and unsigned.

Wallpaper/acrylic, the photo control and export/import were cancelled by the
owner. They are not pending implementation tasks.
