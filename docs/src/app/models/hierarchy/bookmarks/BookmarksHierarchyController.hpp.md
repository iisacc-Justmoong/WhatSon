# `src/app/models/hierarchy/bookmarks/BookmarksHierarchyController.hpp`

## Role
`BookmarksHierarchyController` projects bookmark folders and bookmark note lists.

## Interface Alignment
- System calendar wiring now targets `ISystemCalendarStore`.
- The bookmark hierarchy now implements `IHierarchyExpansionCapability`, so the shared sidebar chevron bridge can
  commit fold/unfold requests for bookmark bucket rows instead of rolling the LVRS row state back.
- Body-state apply and editor stat-refresh APIs are absent; bookmark note lists remain metadata projections.
