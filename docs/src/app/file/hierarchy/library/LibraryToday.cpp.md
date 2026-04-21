# `src/app/models/file/hierarchy/library/LibraryToday.cpp`

## Implementation Notes

- `matches(...)` is now the shared today-membership predicate used by both full rebuilds and incremental note updates.
- `upsertNote(...)` performs structural no-op detection so unchanged note saves do not bubble up as fake today-bucket
  mutations.
- `removeNoteById(...)` allows pruning one today note without replacing the whole derived vector.
