# Readability review, 2026-10-17

This pass covers native process helpers, the launcher background menu, the
This PC manager, filter dialogs and menu glyph rendering. It is not a complete
security or accessibility review of the project.

## Changes made

| Before | After | Source |
| --- | --- | --- |
| Two copies of Windows argument quoting | One shared helper, checked against the Windows parser | native/Support.cpp |
| Repeated module lookup, process launch and edit-control reads | Named helpers with ownership and return-value comments | native/Support.cpp |
| Background command offsets repeated as `3`, `8`, `9` in dispatch and error handling | One table defines each action, label, glyph and error resource | shell/ShellFolder.cpp |
| Nested ternaries select group properties and menu glyphs | A small switch and explicit command records | shell/ShellFolder.cpp |
| Sort columns constructed separately for defaults and Apply | One `sort_columns` helper | shell/ShellFolder.cpp |
| One dialog callback initializes, decorates, reads and validates | Separate setup and accept functions, with a pending settings copy | native/ViewDialog.cpp |
| Recent filters overwrite the edited object before all controls are read | Read and save a local copy before assigning the result | native/RecentDialog.cpp |
| Nested font and glyph fallback loops | Windows 11 Fluent font only; pixel conversion uses named channels | native/MenuIcons.cpp |
| Manager startup scans older registrations | App's current format only; no importer or migration script | native/ThisPcEntries.cpp |
| Manager scans the complete CLSID collection and recreates a class on Save | Read the This PC namespace, validate ownership in both hives and update current keys in place | native/ThisPcEntries.cpp |

Comments explain why an operation is asynchronous, who owns allocations, why a
cancelled operation must not fall through, and when edited state becomes visible.
They do not narrate straightforward assignments.

## Example: menu dispatch

Before, the numeric offsets had to match another list elsewhere:

```cpp
if (first == 3) return open_this_pc_entry_manager(owner);
if (first == 8 || first == 9) { /* view settings */ }
```

Now the offset selects one command record; its named action drives a switch:

```cpp
const auto* command = background_command(offset);
if (!command) return S_FALSE;
return invoke_background_command(owner, root, command->action);
```

## Example: bitmap conversion

The old code combined alpha conversion and three channels in one expression.
The conversion now names the intermediate values:

```cpp
const auto alpha = 255 - gray;
const auto red = GetRValue(color) * alpha / 255;
const auto green = GetGValue(color) * alpha / 255;
const auto blue = GetBValue(color) * alpha / 255;
bits[index] = (alpha << 24) | (red << 16) | (green << 8) | blue;
```

## Remaining candidates

1. **Separate registry storage from the This PC dialog.** `ThisPcEntries.cpp`
   still contains registry ownership, read/write operations, elevation and UI.
   A registry component would make failure-path testing possible without opening
   a dialog. Saving now preserves the existing class and validates both hives
   first, but a later write failure can still leave the copies out of sync.
   There is no cross-hive transaction or rollback yet.
2. **Reuse collection metadata within one operation.** `ShellFolder::detail`
   obtains view preferences and parses all family definitions for each family
   cell. Target extension and date helpers can also reopen the same shortcut.
   A per-operation snapshot could reduce repeated I/O. This is a source-level
   observation, not a measured performance improvement; refresh must still pick
   up newly edited preferences.
3. **Make preference serialization easier to read.** `save_view_settings` still
   combines serialization, HANDLE management, flush and atomic replacement.
   Split those responsibilities while retaining the current corruption and
   target-preservation checks.

## Compatibility policy

The application supports its current data and registration format only. No
importer, migration script or older font fallback is retained.

COM interface inheritance, both documented Shell invocation messages, reference
ownership checks and safe handling of malformed preferences remain. These serve
the current product's contract and data integrity.

Build and runtime evidence is recorded in [VALIDATION.md](VALIDATION.md).
