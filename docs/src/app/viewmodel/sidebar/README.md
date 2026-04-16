# `src/app/viewmodel/sidebar`

## Role
This directory contains the viewmodel layer that coordinates which hierarchy domain is currently active in the sidebar.

It sits above concrete hierarchy domains but below the visual sidebar QML. Its job is routing and normalization, not persistence-heavy mutation.

## Important Types
- `SidebarHierarchyViewModel`: exposes the active hierarchy index and resolves the active hierarchy and note-list models.
- `IActiveHierarchyContextSource`: exposes the active hierarchy index plus the active hierarchy/note-list binding
  snapshot for consumers that need more than activation alone.
- `IHierarchyViewModelProvider` and `HierarchyViewModelProvider`: map sidebar domain indices to dedicated hierarchy viewmodels.
- `HierarchySidebarDomain.hpp`: shared constants and index normalization helpers.

## Why This Layer Exists
The sidebar should not know how to discover the concrete viewmodel for each domain. It should ask one coordinator for:
- the active domain index
- the active hierarchy viewmodel
- the active note-list model

That coordination is exactly what this directory provides.

`HierarchyViewModelProvider` now stores those bindings as index-addressable `Mapping` entries rather than one
hard-coded `Targets` struct field per domain, so the provider no longer needs a switch statement or one member per
hierarchy type just to resolve the active module.
