# `src/app/viewmodel/sidebar/IActiveHierarchyContextSource.hpp`

## Role
`IActiveHierarchyContextSource` extends `IActiveHierarchySource` with the active hierarchy viewmodel and active
note-list model.

## Why It Exists
- Startup/runtime code can continue to depend on the smaller `IActiveHierarchySource` contract when it only needs the
  active index.
- Consumers such as the detail-panel binder can depend on this richer interface instead of the concrete
  `SidebarHierarchyViewModel` type.

## Exposed Contract
- `activeHierarchyIndex()`
- `activeHierarchyViewModel()`
- `activeNoteListModel()`
- `activeBindingsChanged()`, `activeHierarchyViewModelChanged()`, `activeNoteListModelChanged()`
