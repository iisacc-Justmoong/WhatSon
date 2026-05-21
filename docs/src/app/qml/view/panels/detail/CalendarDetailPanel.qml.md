# `src/app/qml/view/panels/detail/CalendarDetailPanel.qml`

## Responsibility
`CalendarDetailPanel.qml` is the dedicated detail-column surface used while a calendar page is mounted in the content
view.

## Current State
- The view is intentionally blank for now.
- It owns a separate calendar-detail surface so calendar-specific detail UI can be added later without branching back
  into `NoteDetailPanel.qml` or `ResourceDetailPanel.qml`.

## Contract
- `signal viewHookRequested`
- `function requestViewHook(reason)`
