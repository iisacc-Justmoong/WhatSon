# `src/app/qml/view/panels/detail/NoteDetailPanel.qml`

## Responsibility
`NoteDetailPanel.qml` is the concrete note-detail surface that used to live directly inside `DetailPanel.qml`.
It owns the note-detail toolbar, the active state/content resolution, and the shared `DetailContents.qml` form mount.

## View-Model Contract
- `property var noteDetailPanelViewModel`
- `activeContentViewModel`
- `fileStatViewModel`
- `projectSelectionViewModel`
- `bookmarkSelectionViewModel`
- `progressSelectionViewModel`
- `activeStateName`
- `noteContextLinked`
- `toolbarItems`

## Behavior
- The component keeps the centered toolbar and the LVRS-scaled content sizing rules from the former monolithic detail
  panel.
- Toolbar clicks still forward to `requestStateChange(stateValue)` on the injected note-detail viewmodel.
- `linked` shows the toolbar plus `DetailContents.qml`.
- `detached` keeps the note-detail surface empty so stale note metadata cannot render when no note context is bound.
- Default panel width and panel spacing use named `LV.Theme` tokens rather than scaled pixel literals.
