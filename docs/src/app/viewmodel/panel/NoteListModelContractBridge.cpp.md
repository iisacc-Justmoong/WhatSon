# `src/app/viewmodel/panel/NoteListModelContractBridge.cpp`

## Responsibility

The implementation provides runtime-safe reads/writes for search text and current-index selection against
heterogeneous note-list model objects.

## Behavior Summary

- Resolves the effective note-list model from either:
  - an explicit `noteListModel`, or
  - the bound `hierarchyViewModel`'s `hierarchyNoteListModel` / `noteListModel` contract.
- Uses `QMetaObject` reflection to detect whether the injected model supports:
  - writable `searchText` property or `setSearchText(QString)`
  - readable/writable `currentIndex` property or `setCurrentIndex(int)`
  - readable `currentNoteId` property
- Reads per-row note ids through the generic `QAbstractItemModel` role map (`noteId`, then `id`) so QML multi-selection
  can batch actions without knowing the concrete list-model type.
- Exposes `currentNoteEntry()` / `readCurrentNoteEntry()` by preferring a model-owned `currentNoteEntry` property,
  then falling back to the current row snapshot and finally to legacy `currentNoteId/currentNoteDirectoryPath`
  properties. This gives note-backed hierarchies the same explicit "current entry" contract shape that resources use
  through `currentResourceEntry`.
- Exports `readAllRows()` / `readAllRowsForModel(QObject*)` snapshots by walking row role names into `QVariantMap`
  payloads, but intentionally skips the full `bodyText` role. `ListBarLayout.qml` only renders
  note-card/resource-card summary fields, so body-only editor mutations no longer perturb the visible row signature or
  trigger avoidable list snapshot churn.
- Connects to `currentIndexChanged()`, `currentNoteEntryChanged()`, and `currentNoteIdChanged()` when available so
  index-driven selection changes rematerialize both the current note id and the current note entry without waiting for
  a hierarchy swap or a second selection writer.
- Forces resolved hierarchy/viewmodel-owned QObject instances back to `QQmlEngine::CppOwnership`, so QML rebinding does
  not attempt to garbage-collect member-owned C++ note-list models during hierarchy switches.
- Clears capability flags automatically when the bound model is destroyed.

## Tests

The maintained C++ regression suite now locks these bridge contracts:
- immediate hierarchy-viewmodel -> note-list-model rebinding without waiting for an extra event turn
- explicit note-list-model overrides must switch exported row snapshots immediately, including folder/tag metadata,
  even if the hierarchy-viewmodel input is changing in the same transition window
- property-backed search/current-index/current-note contracts on hierarchy-owned note-list models
- QML ownership stabilization for hierarchy-owned note-list models during domain switches
- multi-selection index -> note-id resolution against every note-list model that feeds `ListBarLayout.qml`
- current-entry propagation from `LibraryNoteListModel` into the bridge's public `currentNoteEntry` contract, so the
  note list can mimic the resources hierarchy's working explicit-selection pattern
- `readAllRows()` must preserve role-name keyed row snapshots for library/bookmarks/resources note-list models so the
  QML list surface can suppress equivalent refresh flicker while continuing to omit non-visible payload like
  `bodyText`
- `readAllRowsForModel(QObject*)` must remain usable during hierarchy swaps so `ListBarLayout.qml` can read the first
  incoming snapshot without waiting for bridge rebinding order
