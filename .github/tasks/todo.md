# Open engineering work

Only open tasks belong here. Each task has an observable completion condition.
Move implemented work to [done.md](done.md), then to [archive.md](archive.md) after
delivery. Keep remaining validation open.

## MCS-060 - Complete Explorer interaction acceptance

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
  keys. The current workstation has an old per-user development registration;
  its cleanup was rejected by automatic approval review and was not bypassed.
- [ ] Validate ARM64 runtime on native ARM64 Windows 11. Cross-compilation and
  MSI ICE checks alone do not satisfy this task.
- [ ] Record a source/security/accessibility review before a release. The owner
  selects a version; 0.0.0 packages remain development-only and unsigned.

Wallpaper/acrylic, the photo control and export/import were cancelled by the
owner. They are not pending implementation tasks.
