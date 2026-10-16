# Implemented work awaiting delivery

## MCS-080 - Readability and current-format cleanup

- Consolidated native helpers and background-menu metadata; expanded dialog/drop
  flow and added comments for ownership, validation and asynchronous launch.
- Removed the old importer, its staged migration script and the older font
  fallback. The function belongs entirely to the app's current format.
- Examples and remaining candidates are in [the review](../../docs/CODE_REVIEW.md).
- Local verification is recorded in [validation](../../docs/VALIDATION.md).
- Local installation and actual Explorer acceptance remain pending.

The installed prototype and bootstrap are recorded in [archive.md](archive.md).
Remaining acceptance and installer/architecture checks stay in [todo.md](todo.md).
