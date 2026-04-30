# `src/app/qml/view/panels/detail/ResourceDetailPanel.qml`

## Responsibility
`ResourceDetailPanel.qml` is the dedicated detail-column surface for the resources hierarchy.

## Current State
- The view is intentionally blank for now.
- It already accepts its own `resourceDetailPanelController` contract so resource-specific detail UI can be added
  later without branching back into the note-detail surface.

## Contract
- `property var resourceDetailPanelController`
- `signal viewHookRequested`
- `function requestViewHook(reason)`
