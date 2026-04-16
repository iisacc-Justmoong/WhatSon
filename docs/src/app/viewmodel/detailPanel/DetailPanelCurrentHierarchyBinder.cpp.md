# `src/app/viewmodel/detailPanel/DetailPanelCurrentHierarchyBinder.cpp`

## Runtime Behavior
- Connects to `activeBindingsChanged()` from the injected hierarchy context source.
- Synchronizes `DetailPanelViewModel::setCurrentNoteListModel(...)` and
  `DetailPanelViewModel::setCurrentNoteDirectorySourceViewModel(...)` as one composed binding step.
- Resets the detail-panel context to `nullptr` models when the hierarchy source is destroyed.
