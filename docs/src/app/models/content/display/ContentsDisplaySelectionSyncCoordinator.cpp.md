# `src/app/models/content/display/ContentsDisplaySelectionSyncCoordinator.cpp`

## Responsibility

Coordinates selection-driven note-body snapshot sync between the active editor session and the selected note snapshot.

## Key Behavior

- Builds selection-sync flush plans for note switches, focus restore, cache reset, and delayed snapshot availability.
- Builds snapshot poll/reconcile plans for same-note background comparison without violating local editor authority.
- Snapshot plans now always carry the current `selectedNoteId`, body-note id, resolved flag, and current body text in the
  returned payload even when the action is blocked.
  This keeps trace output truthful and lets the QML host diagnose why parser mounting is blocked without losing the
  selected-note context.
- The coordinator still gates reconcile attempts on:
  - selected note visibility
  - editor session binding to the selected note
  - resolved body ownership for that same note id
  - no pending local typing/save protection

## Why It Exists

The editor host must distinguish between “no usable body yet”, “local edits own the truth”, and “safe to refresh the
snapshot now”. This coordinator centralizes that policy so QML only executes one explicit selection-sync plan at a
time.
