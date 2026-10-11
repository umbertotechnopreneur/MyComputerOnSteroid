# Open engineering work

Only open tasks belong here. Each task has an observable completion condition.
Move implemented work to [done.md](done.md), then to [archive.md](archive.md) after
delivery. Keep remaining validation open.

## MCS-010 - Verify the COM bootstrap in isolation

- [ ] Check factory identity, reference counting, locks, interface ownership,
  empty enumeration and PIDL clone ownership without registering the provider.
- [ ] Verify DefView construction on Windows 11 and record the actual build and
  architecture. A returned interface is not an installed UI check.

## MCS-020 - Reversible Explorer registration

- [ ] Implement per-user registration for the single launcher CLSID, with a
  matching uninstall that removes only keys owned by this project.
- [ ] In an explicitly authorized environment, open the launcher in Explorer,
  verify unload/update behavior and prove registration can be removed cleanly.

## MCS-030 - Items, groups and native views

- [ ] Implement drive enumeration and favorite references for folders, apps and
  files. Use synthetic entries during validation, not a personal inventory.
- [ ] Supply groups and columns through Shell contracts; verify icons, list and
  details modes, empty sections and normal sorting. No manual ordering.
- [ ] Check x64 and ARM64 runtime behavior separately, including DPI and themes.

## MCS-040 - Menus and favorite mutations

- [ ] Record Windows 11 compact-menu and Show more options behavior for the
  background, one shortcut, one drive and mixed selections inside actual Explorer.
- [ ] Implement drop-to-add, duplicate policy and unavailable-target behavior.
- [ ] Implement Remove from launcher for favorite references only. Prove target
  files remain intact and system drives cannot be removed.
- [ ] Add launcher-only icon overrides and a native icon file picker that remembers
  its last usable directory. Record accessibility and keyboard behavior.

## MCS-050 - Background decision gate

- [ ] Determine whether any supported approach meets the background requirement
  without losing the accepted Shell/menu behavior. Record the actual experiment.
- [ ] If a custom view is chosen, keep it native C++ and separately validate its
  menu behavior. The photo icon is unframed, at the bottom right, with a tooltip
  and accessible name. Do not ship private Explorer hooks as a silent fallback.
