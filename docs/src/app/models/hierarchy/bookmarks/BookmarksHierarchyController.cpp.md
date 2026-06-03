# `src/app/models/hierarchy/bookmarks/BookmarksHierarchyController.cpp`

## Implementation Notes
- `setSystemCalendarStore(...)` now binds through `ISystemCalendarStore`.
- Bookmark note list refresh still occurs when locale/date-format state changes.
- When the hierarchy has visible rows, a negative or invalid selected index is normalized to the first visible row
  before the bookmark note list is refreshed, keeping the filter aligned with the sidebar's active row.
- Bookmark row projection is metadata-only. Body-state apply and editor stat-refresh hooks were removed with the note
  editor/save boundary.
- `setItemExpanded(...)` delegates shared chevron validation/state flipping to `IHierarchyController`, then republishes
  the bookmark projection through `syncModel()`. This keeps bookmark bucket rows on the same single-row chevron
  expansion contract as the other hierarchy domains.
