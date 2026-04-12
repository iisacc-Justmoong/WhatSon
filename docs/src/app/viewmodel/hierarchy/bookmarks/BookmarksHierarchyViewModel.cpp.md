# `src/app/viewmodel/hierarchy/bookmarks/BookmarksHierarchyViewModel.cpp`

## Implementation Notes
- `setSystemCalendarStore(...)` now binds through `ISystemCalendarStore`.
- Bookmark note list refresh still occurs when locale/date-format state changes.
- Direct editor writes now update the bookmarked note record through `applyPersistedBodyStateForNote(...)` first and
  defer `.wsnbody` backlink/open-count scans to `requestTrackedStatisticsRefreshForNote(...)`.
- Bookmark row projection no longer carries the full note body into the bookmark note-list model.
  Summary/search text still comes from indexed note metadata, while the editor now lazy-loads the selected note body
  through the selection bridge.
