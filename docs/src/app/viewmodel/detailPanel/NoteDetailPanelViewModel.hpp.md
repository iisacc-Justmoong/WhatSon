# `src/app/viewmodel/detailPanel/NoteDetailPanelViewModel.hpp`

## Responsibility
`NoteDetailPanelViewModel` is the concrete runtime type registered into QML for note-backed detail panels.

## Structure
- Inherits `DetailPanelViewModel` without changing its behavior.
- Exists so the runtime can mount an explicitly note-scoped detail viewmodel alongside the separate
  `ResourceDetailPanelViewModel`.

## Runtime Role
- `main.cpp` instantiates `NoteDetailPanelViewModel` for every non-resource hierarchy.
- `DetailPanel.qml` routes that object into `NoteDetailPanel.qml`.
