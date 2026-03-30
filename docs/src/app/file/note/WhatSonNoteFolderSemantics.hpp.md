# `src/app/file/note/WhatSonNoteFolderSemantics.hpp`

## Responsibility
This header centralizes lightweight folder-path rules that must stay consistent across note-header parsing, mutation, and detail-panel presentation.

## Public Helpers
- `normalizeFolderPath(QString)` normalizes slash direction, trims whitespace, and removes leading or trailing separators.
- `leafFolderName(QString)` returns only the final visible segment of a normalized folder path.
  - Example: `Research/Ideas` becomes `Ideas`.
- `usesReservedTodayFolderSegment(const QString&)` guards the reserved `Today` segment policy.
- `inspectRawFoldersBlock(const QString&)` inspects whether a raw `.wsnhead` folders block exists and whether it contains a concrete entry.

## Notes
- The helper does not change persistence. `.wsnhead` and `.wsfolders` continue to store normalized full paths.
- Detail-panel folder presentation uses `leafFolderName(...)` for user-facing labels while `.wsnhead` and `.wsfolders` persistence stays on normalized full paths.
