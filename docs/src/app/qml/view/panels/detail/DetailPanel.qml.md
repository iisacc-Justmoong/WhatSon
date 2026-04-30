# `src/app/qml/view/panels/detail/DetailPanel.qml`

## Responsibility
`DetailPanel.qml` is now the hierarchy-aware router for the right-hand detail column.
It no longer renders the note-detail form directly. Instead it decides whether the active hierarchy is the dedicated
resources hierarchy and mounts one of two concrete surfaces:

- `NoteDetailPanel.qml`
- `ResourceDetailPanel.qml`

## Key Contracts
- `noteDetailPanelController`
- `resourceDetailPanelController`
- `sidebarHierarchyController`
- `resourcesHierarchyController`

## Behavior
- When the active hierarchy controller matches `resourcesHierarchyController`, the router hides the note-detail surface
  and shows `ResourceDetailPanel.qml`.
- Every other hierarchy continues to mount `NoteDetailPanel.qml`.
- The outer `DetailPanel` shell remains stable for `RightPanel.qml` and the page layouts even though the mounted view
  and effective detail controller now diverge by hierarchy domain.
