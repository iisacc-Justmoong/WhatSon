# `src/app/viewmodel/hierarchy/event/EventHierarchyViewModel.hpp`

## Responsibility

This header declares the dedicated event hierarchy viewmodel. It presents the `Event.wsevent`
taxonomy as an LVRS-compatible hierarchy model and exposes the mutable hooks required by sidebar
views.

## Public Contract

- Publishes the row model, selection index, item count, and load-state properties.
- Implements rename, create, delete, and expansion capabilities on top of
  `IHierarchyViewModel`.
- Exposes `setEventNames(...)` for direct data injection and `applyRuntimeSnapshot(...)` for
  runtime-loader driven refreshes.
- Exposes `setItemExpanded(int, bool)` so fold state belongs to the viewmodel instead of to a
  transient delegate instance.

## Refresh Rules

- Snapshot application must preserve the current selection and expansion state when the event
  hierarchy is rebuilt.
- Snapshot application must skip the rebuild entirely when the sanitized event-name list is
  unchanged.
- Load failures update the error surface without destroying the currently visible hierarchy.

## Internal State

- `m_eventNames` is the current canonical event-name payload.
- `m_items` is the rendered bucket/item hierarchy consumed by `EventHierarchyModel`.
- `m_store` remains the serializer/parser-facing domain store for `Event.wsevent`.
- `m_eventFilePath` identifies the mutation target used by rename/create/delete operations.
