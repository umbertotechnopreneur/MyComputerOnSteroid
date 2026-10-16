# Code cleanup, 2026-10-17

| Before | Now |
| --- | --- |
| The same helper in two files | One shared helper |
| Menu actions picked with numbers like `3` and `8` | Named actions in one table |
| Long dialog callbacks | Separate setup, read and check steps |
| Many calculations on one line | Named values and more space |
| Old import and font fallback code | Current app format and Fluent font only |

Comments explain who owns resources, when data changes and what happens on errors.
They do not repeat obvious code.

For example, the color calculation now has clear steps:

```cpp
const auto alpha = 255 - gray;
const auto red = GetRValue(color) * alpha / 255;
const auto green = GetGValue(color) * alpha / 255;
const auto blue = GetBValue(color) * alpha / 255;
```

Still worth improving:

- Separate registry storage from the dialog. A failed write can leave the user
  and machine copies different; there is no rollback across both.
- Read shared metadata once per operation. This could reduce disk reads.
- Split settings-file writes into smaller methods.

No importer or migration script is kept. Data checks and current Shell contracts
stay in place.

Debug and Release x64 checks passed. Release passed 259 assertions.
The new dialogs and registry writes still need live Explorer checks.
[Validation](VALIDATION.md) has the details.
