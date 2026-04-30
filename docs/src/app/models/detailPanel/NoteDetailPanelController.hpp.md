# `src/app/models/detailPanel/NoteDetailPanelController.hpp`

## Responsibility
`NoteDetailPanelController` is the concrete runtime type registered into QML for note-backed detail panels.

## Structure
- Inherits `DetailPanelController` without changing its behavior.
- Exists so the runtime can mount an explicitly note-scoped detail controller alongside the separate
  `ResourceDetailPanelController`.

## Runtime Role
- `main.cpp` instantiates `NoteDetailPanelController` for every non-resource hierarchy.
- `DetailPanel.qml` routes that object into `NoteDetailPanel.qml`.
