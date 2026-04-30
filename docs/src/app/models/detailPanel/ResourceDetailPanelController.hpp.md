# `src/app/models/detailPanel/ResourceDetailPanelController.hpp`

## Responsibility
`ResourceDetailPanelController` is the dedicated detail-panel controller for the resources hierarchy.

## Exported Contract
- `resourceListModel`
- `resourceContextLinked`
- `currentResourceEntry`
- `setCurrentResourceListModel(QObject*)`

## Notes
- The type intentionally keeps a narrow contract because the matching `ResourceDetailPanel.qml` surface is currently
  blank.
- The class still exists now so the resource-detail route can diverge structurally from note detail without sharing
  note-specific selectors or toolbar state.
