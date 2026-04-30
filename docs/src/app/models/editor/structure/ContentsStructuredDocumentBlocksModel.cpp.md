# `src/app/models/content/ContentsStructuredDocumentBlocksModel.cpp`

## Responsibility
Implements row-preserving diff application for parsed structured note blocks.

## Current Behavior
- Normalizes incoming `QVariantList` block payloads into retained model entries.
- Computes common prefix/suffix matches from stable semantic keys so offset-only shifts in later blocks stay on the
  existing rows.
- Applies middle-row insert/remove operations only where the parsed block stream actually changed.
- Emits contiguous `dataChanged` ranges for retained rows whose payload changed after the edit.
- Avoids `beginResetModel()` / `endResetModel()` for ordinary structured editor mutations such as single-line delete.
