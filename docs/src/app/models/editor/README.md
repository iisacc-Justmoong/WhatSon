# Editor Model Helpers

This shard now contains standalone editor helper code only.

## Remaining Scope

- `GetProperty.*`, `SetProperty.*`, `SetTag.*`, and `GetTag.*` provide parser/mutation helpers.
- `TagInsertionWriter.*` persists explicit tag insertion requests through the note file store.
- `EditorFontFamilyProvider.*` exposes available font-family menu data.
- `component/*` contains isolated source/projection helper components used by tests and legacy helper paths.

## Deleted Boundary

The active editor document session, editor native input filter, and editor paste bridge were removed. Do not reintroduce those responsibilities in this shard without defining a new document model contract first.
