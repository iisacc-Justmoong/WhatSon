# `src/app/viewmodel/hierarchy/library/LibraryHierarchyViewModel.cpp`

## Implementation Notes
- `setSystemCalendarStore(...)` now binds to `ISystemCalendarStore` and its `systemInfoChanged` signal.
- Note-list date formatting behavior is unchanged.
- Static `SystemCalendarStore::formatNoteDateForSystem(...)` remains the non-injected fallback helper.
