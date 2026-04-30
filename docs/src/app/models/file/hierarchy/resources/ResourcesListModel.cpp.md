# `src/app/models/file/hierarchy/resources/ResourcesListModel.cpp`

## Responsibility

Implements filtering and selection state for the Resources right-panel list model.

## Key Behavior

- Accepts resource-domain list items through `setItems(...)`.
- Normalizes list payload fields (`id`, metadata lists, image sources, searchable text).
- Compares the sanitized incoming source cache against the existing cache before replacing it, so identical resource
  list refreshes stop before `beginResetModel()/endResetModel()` and `itemsChanged()` churn.
- Supports search filtering via `searchText` (case-folded term matching).
- Preserves selection by `currentNoteId` across filter and reset operations when possible.
- Retains the selected resource id when the item payload is replaced and the same id still exists.
- Publishes `noteBacked == false` explicitly.
  Selection/id values therefore remain available to generic list delegates, but note-specific bridges must treat this
  model as a resource browser rather than a real note source.
- Mirrors the selected row into `currentResourceEntry` (`type`, `format`, `resourcePath`, `resolvedPath`,
  `source`, `renderMode`, `displayName`, `previewText`) for dedicated resource viewer pipelines.
- Emits shared selection signals required by:
  - `NoteListModelContractBridge`
  - `ListBarLayout`
  - note-agnostic selection consumers only

## Compatibility

The model keeps note-card role names required by existing QML delegates, while remaining an isolated
resource-domain implementation.
