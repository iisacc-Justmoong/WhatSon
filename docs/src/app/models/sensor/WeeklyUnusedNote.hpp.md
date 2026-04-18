# `src/app/models/sensor/WeeklyUnusedNote.hpp`

## Responsibility

`WeeklyUnusedNote` is the fixed-window sensor object for notes that have not been opened for at least one week.

## Surface

- `hubPath`: unpacked `.wshub` root to inspect.
- `unusedNotes`: full entry payloads including timestamps and note paths.
- `unusedNoteIds`: convenience projection of the filtered note ids.
- `unusedNoteCount`: count projection for bindings.
- `lastError`: validation or scan failure text.

## Event Model

- Emits `unusedNotesChanged()` whenever the filtered result changes.
- Emits `scanCompleted(...)` after every refresh, including empty-state refreshes.
- Exposes `refresh()` as the required slot entrypoint for the model domain.
