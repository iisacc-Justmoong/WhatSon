# `src/app/file/note/WhatSonNoteFolderSemantics.hpp`

## Responsibility
This header centralizes lightweight folder-path rules that must stay consistent across note-header parsing, mutation, and detail-panel presentation.

## Public Helpers
- `escapeFolderPathSegment(QString)` escapes literal `\` and `/` characters inside one folder label.
- `folderPathSegments(QString)` splits a persisted folder path into logical segments while treating `\/` as a literal slash and `\\` as a literal backslash.
- `joinFolderPathSegments(QStringList)` rebuilds the canonical persisted form from logical segments.
- `normalizeFolderPath(QString)` canonicalizes a stored path into the escaped segment form used by `.wsfolders` / note-folder bindings.
- `appendFolderPathSegment(parent, label)` appends one literal folder label to an existing persisted path without letting `/` inside that label create accidental child hierarchy.
- `displayFolderPath(QString)` returns a user-facing path string with escaped separators decoded back to plain text.
- `isHierarchicalFolderPath(QString)` distinguishes a true multi-segment hierarchy path from a single folder label that merely contains a literal slash.
- `leafFolderName(QString)` returns only the final visible segment of a normalized folder path.
  - Example: `Research/Ideas` becomes `Ideas`.
- `usesReservedTodayFolderSegment(const QString&)` guards the reserved `Today` segment policy.
- `inspectRawFoldersBlock(const QString&)` inspects whether a raw `.wsnhead` folders block exists and whether it contains a concrete entry.

## Notes
- Persistence now treats `/` inside one folder label as escaped content (`\/`) instead of as an unconditional hierarchy separator.
- `.wsnhead` and `.wsfolders` still store normalized full paths, but those paths are now segment-escaped rather than raw slash-joined labels.
- Detail-panel folder presentation uses `leafFolderName(...)` / `displayFolderPath(...)` so user-facing labels keep literal `/` characters without exposing the persisted escape markers.
