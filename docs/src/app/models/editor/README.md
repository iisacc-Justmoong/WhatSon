# Editor Model Helpers

This shard now contains standalone editor helper code only.

## Remaining Scope

- `GetProperty.*`, `SetProperty.*`, `SetTag.*`, and `GetTag.*` provide parser/mutation helpers.
- `EditorFontFamilyProvider.*` exposes available font-family menu data.
- `component/*` contains isolated source/projection helper components used by tests and legacy helper paths.

## Deleted Boundary

The active editor document session, editor native input filter, editor paste bridge, and persisted tag insertion writer
were removed. Do not reintroduce those responsibilities in this shard without defining a new document model contract
first.
