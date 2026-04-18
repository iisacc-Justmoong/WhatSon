# `src/app/models/sensor/MonthlyUnusedNote.hpp`

## Responsibility

`MonthlyUnusedNote` is the fixed-window sensor object for notes that have not been opened for at least one month.

## Surface

- Mirrors `WeeklyUnusedNote` so QML/C++ callers can swap only the period object.
- Returns both the rich `unusedNotes` payload and the stripped `unusedNoteIds` list.
