# `src/app/viewmodel/sidebar`

## Role
This directory contains the viewmodel layer that coordinates which hierarchy domain is currently active in the sidebar.

It sits above concrete hierarchy domains but below the visual sidebar QML. Its job is routing and normalization, not persistence-heavy mutation.

## Important Types
- `SidebarHierarchyViewModel`: exposes the active hierarchy index and resolves the active hierarchy and note-list models.
- `IHierarchyViewModelProvider` and `HierarchyViewModelProvider`: map sidebar domain indices to dedicated hierarchy viewmodels.
- `HierarchySidebarDomain.hpp`: shared constants and index normalization helpers.

## Why This Layer Exists
The sidebar should not know how to discover the concrete viewmodel for each domain. It should ask one coordinator for:
- the active domain index
- the active hierarchy viewmodel
- the active note-list model

That coordination is exactly what this directory provides.
