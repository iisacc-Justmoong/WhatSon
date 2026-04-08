# `src/app/viewmodel/hierarchy/bookmarks/BookmarksHierarchyViewModel.hpp`

## Role
`BookmarksHierarchyViewModel` projects bookmark folders and bookmark note lists.

## Interface Alignment
- System calendar wiring now targets `ISystemCalendarStore`.
- The rest of the bookmark hierarchy capability surface is unchanged.
- The editable bookmark note surface now also exposes `applyPersistedBodyStateForNote(...)` and
  `requestTrackedStatisticsRefreshForNote(...)` so editor autosave can use the same split
  `cheap body sync -> deferred stat refresh` contract as the library domain.
