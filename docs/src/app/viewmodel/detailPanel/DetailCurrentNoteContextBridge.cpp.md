# `src/app/viewmodel/detailPanel/DetailCurrentNoteContextBridge.cpp`

## Responsibility

Keeps the detail panel's current note id and note-directory context synchronized with the active right-panel list model
and the active hierarchy view model.

## Key Behavior

- Reads `currentNoteId` from the active list model when that model is genuinely note-backed.
- If the active list model exposes `noteBacked == false`, the bridge now treats that model as a non-note browser and
  resolves an explicit empty note context.
- This prevents resource-domain list models from being interpreted as real note packages just because they reuse the
  shared `currentNoteId` property name for generic list selection.
- Directory resolution still routes through `noteDirectoryPathForNoteId(QString)` only when a real note id is present.

## Regression Checks

- Switching into the Resources hierarchy must clear detail-panel note-header context instead of trying to load
  `.wsnhead` metadata from a `.wsresource` package directory.
- Switching back to Library must rebuild the note context from the library note-list model on the next refresh turn.
