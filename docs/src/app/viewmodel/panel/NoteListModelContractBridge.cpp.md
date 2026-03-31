# `src/app/viewmodel/panel/NoteListModelContractBridge.cpp`

## Responsibility

The implementation provides runtime-safe reads/writes for search text and current-index selection against
heterogeneous note-list model objects.

## Behavior Summary

- Uses `QMetaObject` reflection to detect whether the injected model supports:
  - writable `searchText` property or `setSearchText(QString)`
  - readable/writable `currentIndex` property or `setCurrentIndex(int)`
  - readable `currentNoteId` property
- Connects to `currentIndexChanged()` and `currentNoteIdChanged()` when available to keep QML bindings reactive.
- Clears capability flags automatically when the bound model is destroyed.

## Tests

- `tests/app/test_note_list_model_contract_bridge.cpp` validates:
  - property-backed contracts,
  - method-only contracts,
  - model-destroy lifecycle reset behavior.
