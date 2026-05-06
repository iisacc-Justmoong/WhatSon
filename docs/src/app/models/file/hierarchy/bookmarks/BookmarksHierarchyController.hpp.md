# `src/app/models/file/hierarchy/bookmarks/BookmarksHierarchyController.hpp`

## Role
`BookmarksHierarchyController` projects bookmark folders and bookmark note lists.

## Interface Alignment
- System calendar wiring now targets `ISystemCalendarStore`.
- The bookmark hierarchy now implements `IHierarchyExpansionCapability`, so the shared sidebar chevron bridge can
  commit fold/unfold requests for bookmark bucket rows instead of rolling the LVRS row state back.
- The editable bookmark note surface now also exposes `applyPersistedBodyStateForNote(...)` and
  `requestTrackedStatisticsRefreshForNote(...)` so editor autosave can use the same split
  `cheap body sync -> deferred stat refresh` contract as the library domain.
