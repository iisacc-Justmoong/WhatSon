# `src/app/viewmodel/panel/NoteListModelContractBridge.cpp`

## Responsibility

The implementation provides runtime-safe reads/writes for search text and current-index selection against
heterogeneous note-list model objects.

## Behavior Summary

- Uses `QMetaObject` reflection to detect whether the injected model supports:
  - writable `searchText` property or `setSearchText(QString)`
  - readable/writable `currentIndex` property or `setCurrentIndex(int)`
  - readable `currentNoteId` property
- Reads per-row note ids through the generic `QAbstractItemModel` role map (`noteId`, then `id`) so QML multi-selection
  can batch actions without knowing the concrete list-model type.
- Exports `readAllRows()` snapshots by walking row role names into `QVariantMap` payloads, but intentionally skips the
  full `bodyText` role. `ListBarLayout.qml` only renders note-card/resource-card summary fields, so body-only editor
  mutations no longer perturb the visible row signature or trigger avoidable list snapshot churn.
- Connects to `currentIndexChanged()` and `currentNoteIdChanged()` when available to keep QML bindings reactive.
- Clears capability flags automatically when the bound model is destroyed.

## Tests

Automated test files were removed from this repository; keep these bridge contracts stable:
- property-backed contracts
- method-only contracts
- model-destroy lifecycle reset behavior
- multi-selection index -> note-id resolution against every note-list model that feeds `ListBarLayout.qml`
- `readAllRows()` must preserve role-name keyed row snapshots for library/bookmarks/resources note-list models so the
  QML list surface can suppress equivalent refresh flicker while continuing to omit non-visible payload like
  `bodyText`
