# `src/app/viewmodel/hierarchy/library/LibraryHierarchyViewModel.cpp`

## Implementation Notes
- `setSystemCalendarStore(...)` now binds to `ISystemCalendarStore` and its `systemInfoChanged` signal.
- Note-list date formatting behavior is unchanged.
- Static `SystemCalendarStore::formatNoteDateForSystem(...)` remains the non-injected fallback helper.
- `createFolder()` remains the authoritative library-folder creation path. When a non-protected folder is selected, it
  computes the insertion point after that folder's subtree, increases depth by one, expands the parent, and therefore
  creates the new folder as a child of the selected folder.
- `deleteSelectedFolder()` remains the authoritative delete path. It removes the selected folder together with its
  descendant subtree and persists the updated folders store before refreshing sidebar state.
- The library sidebar right-click context menu now reuses those two existing methods through
  `HierarchyInteractionBridge`; no separate library-specific CRUD implementation was added for the menu.
