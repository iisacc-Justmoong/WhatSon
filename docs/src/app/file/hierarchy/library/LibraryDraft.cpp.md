# `src/app/file/hierarchy/library/LibraryDraft.cpp`

## Implementation Notes

- `matches(...)` is now the shared draft-membership predicate used by both full rebuilds and incremental note updates.
- `upsertNote(...)` performs structural no-op detection so a body save that does not actually change the stored draft
  record does not propagate a fake "changed" signal upstream.
- `removeNoteById(...)` allows draft-bucket pruning without replacing the entire derived vector.
