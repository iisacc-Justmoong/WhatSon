# `src/app/qml/view/panels/detail/DetailPanel.qml`

## Responsibility
`DetailPanel.qml` is now the route-aware router for the right-hand detail column.
It no longer renders the note-detail form directly. Instead it decides whether calendar content is active, whether the
active hierarchy is the dedicated resources hierarchy, and mounts one of three concrete surfaces:

- `CalendarDetailPanel.qml`
- `NoteDetailPanel.qml`
- `ResourceDetailPanel.qml`

## Key Contracts
- `noteDetailPanelController`
- `resourceDetailPanelController`
- `sidebarHierarchyController`
- `resourcesHierarchyController`
- `calendarDetailActive`

## Behavior
- When a calendar page is mounted in the content view, the router shows `CalendarDetailPanel.qml`.
- When the active hierarchy controller matches `resourcesHierarchyController`, the router hides the note-detail surface
  and shows `ResourceDetailPanel.qml`.
- Every other hierarchy continues to mount `NoteDetailPanel.qml`.
- Calendar detail state takes precedence over the resources hierarchy state because the content slot is currently
  calendar-owned.
- The outer `DetailPanel` shell remains stable for `RightPanel.qml` and the page layouts even though the mounted view
  and effective detail controller now diverge by hierarchy domain.
