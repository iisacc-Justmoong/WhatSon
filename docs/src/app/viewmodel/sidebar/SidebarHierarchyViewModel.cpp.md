# `src/app/viewmodel/sidebar/SidebarHierarchyViewModel.cpp`

## Implementation Notes
- Constructor now initializes the `IActiveHierarchySource` base.
- Selection-store and provider behavior are unchanged.
- The new interface is used by deferred startup hierarchy loading.
