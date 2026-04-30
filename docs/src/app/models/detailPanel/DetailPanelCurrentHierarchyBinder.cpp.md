# `src/app/models/detailPanel/DetailPanelCurrentHierarchyBinder.cpp`

## Runtime Behavior
- Connects to `activeBindingsChanged()` from the injected hierarchy context source.
- Synchronizes the active note-list model plus hierarchy directory resolver into `NoteDetailPanelController`.
- Detects when the active hierarchy index is `Resources` and only then forwards the active note-list model into
  `ResourceDetailPanelController::setCurrentResourceListModel(...)`.
- Clears the resource-detail binding again when the active hierarchy switches back to a note-backed domain.
- Resets the detail-panel context to `nullptr` models when the hierarchy source is destroyed.
