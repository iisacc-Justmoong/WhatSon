# `src/app/models/file/hierarchy/bookmarks/BookmarksHierarchyController.cpp`

## Implementation Notes
- `setSystemCalendarStore(...)` now binds through `ISystemCalendarStore`.
- Bookmark note list refresh still occurs when locale/date-format state changes.
- When the hierarchy has visible rows, a negative or invalid selected index is normalized to the first visible row
  before the bookmark note list is refreshed, keeping the filter aligned with the sidebar's active row.
- Direct editor writes now update the bookmarked note record through `applyPersistedBodyStateForNote(...)` first and
  defer `.wsnbody` backlink/open-count scans to `requestTrackedStatisticsRefreshForNote(...)`.
- Bookmark row projection now keeps the selected note's RAW/source snapshot in `BookmarksNoteListItem::bodyText`.
  Summary/search text still comes from indexed note metadata, but the selection bridge can now reuse the bookmark
  note-list model's current-row RAW body immediately before it falls back to content-view-model or filesystem reload
  paths.
- `setItemExpanded(...)` accepts only rows that advertise `showChevron`, updates the bookmark hierarchy item, and
  republishes the model through `syncModel()`. This keeps bookmark bucket rows on the same single-row chevron
  expansion contract as the other hierarchy domains.
