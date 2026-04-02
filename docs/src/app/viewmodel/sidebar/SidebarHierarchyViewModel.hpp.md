# `src/app/viewmodel/sidebar/SidebarHierarchyViewModel.hpp`

## Role
`SidebarHierarchyViewModel` owns sidebar selection state and active hierarchy bindings.

## Interface Alignment
- Now implements `IActiveHierarchySource`.
- This allows startup runtime coordination to observe activation changes without depending on the full sidebar implementation type.
