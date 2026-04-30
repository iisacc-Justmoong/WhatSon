# `src/app/models/sidebar/IActiveHierarchyContextSource.hpp`

## Role
`IActiveHierarchyContextSource` extends `IActiveHierarchySource` with the active hierarchy controller and active
note-list model.

## Why It Exists
- Startup/runtime code can continue to depend on the smaller `IActiveHierarchySource` contract when it only needs the
  active index.
- Consumers such as the detail-panel binder can depend on this richer interface instead of the concrete
  `SidebarHierarchyController` type.

## Exposed Contract
- `activeHierarchyIndex()`
- `activeHierarchyController()`
- `activeNoteListModel()`
- `activeBindingsChanged()`, `activeHierarchyControllerChanged()`, `activeNoteListModelChanged()`
