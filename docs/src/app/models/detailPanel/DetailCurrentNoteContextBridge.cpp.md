# `src/app/models/detailPanel/DetailCurrentNoteContextBridge.cpp`

## Responsibility

Keeps the detail panel's current note id and note-directory context synchronized with the active right-panel list model
and the active hierarchy view model.

## Key Behavior

- Reads the active note context from `currentNoteEntry` first when the active list model is genuinely note-backed.
- Falls back to legacy `currentNoteId/currentNoteDirectoryPath` properties only when the entry contract is unavailable
  or incomplete, so older note-list models remain readable while note-backed hierarchies converge on the same explicit
  current-entry pattern already used by resources.
- When a note-backed list model exposes a readable current-selection contract but that contract is explicitly empty, the
  bridge now clears `currentNoteId/currentNoteDirectoryPath` instead of retaining the previous note context.
- Stale context retention is now reserved for the narrower case where the active model does not expose any readable
  note-selection identity contract at all.
- Listens to `currentIndexChanged()`, `currentNoteEntryChanged()`, `currentNoteIdChanged()`, and
  `currentNoteDirectoryPathChanged()` on the bound note-list model, mirroring the resources detail-panel pattern where
  the current entry is rematerialized from index changes instead of assuming a single property writer will always fire.
- If the active list model exposes `noteBacked == false`, the bridge now treats that model as a non-note browser and
  resolves an explicit empty note context.
- This prevents resource-domain list models from being interpreted as real note packages just because they reuse the
  shared `currentNoteId` property name for generic list selection.
- Directory resolution still routes through `noteDirectoryPathForNoteId(QString)` only when the current entry does not
  already provide `noteDirectoryPath`.

## Regression Checks

- Switching into the Resources hierarchy must clear detail-panel note-header context instead of trying to load
  `.wsnhead` metadata from a `.wsresource` package directory.
- Switching back to Library must rebuild the note context from the library note-list model on the next refresh turn.
- A library note with a populated `currentNoteEntry.noteDirectoryPath` must keep that path even if the source
  controller's legacy resolver returns a different location for the same note id.
- A note-backed model that exposes a readable but empty `currentNoteEntry` must clear the detail-panel note context
  rather than leaving the previous note id/directory path mounted.
