# `src/app/models/content/ContentsStructuredDocumentBlocksModel.hpp`

## Responsibility
Declares the incremental structured-block list model that keeps `ContentsStructuredDocumentFlow.qml` rows stable across
RAW reparses.

## Current Contract
- Exposes parsed document blocks as a QML-consumable `QAbstractListModel`.
- Accepts whole parsed block snapshots through `blocks`.
- Preserves stable rows by matching blocks on a retention key derived from semantic type/tag/source text rather than
  raw source offsets alone.
- Emits ordinary `rowsInserted`, `rowsRemoved`, and `dataChanged` updates instead of forcing the structured editor host
  to rebuild every block delegate on each note edit.
