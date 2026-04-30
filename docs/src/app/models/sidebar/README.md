# `src/app/models/sidebar`

## Role
This directory contains the controller layer that coordinates which hierarchy domain is currently active in the sidebar.

It sits above concrete hierarchy domains but below the visual sidebar QML. Its job is routing and normalization, not persistence-heavy mutation.

## Important Types
- `SidebarHierarchyController`: exposes the active hierarchy index and resolves the active hierarchy and note-list models.
- `IActiveHierarchyContextSource`: exposes the active hierarchy index plus the active hierarchy/note-list binding
  snapshot for consumers that need more than activation alone.
- `IHierarchyControllerProvider` and `HierarchyControllerProvider`: map sidebar domain indices to dedicated hierarchy controllers.
- `HierarchySidebarDomain.hpp`: shared constants and index normalization helpers.

## Why This Layer Exists
The sidebar should not know how to discover the concrete controller for each domain. It should ask one coordinator for:
- the active domain index
- the active hierarchy controller
- the active note-list model

That coordination is exactly what this directory provides.

`HierarchyControllerProvider` now stores those bindings as index-addressable `Mapping` entries rather than one
hard-coded `Targets` struct field per domain, so the provider no longer needs a switch statement or one member per
hierarchy type just to resolve the active module.

Automated C++ regression coverage for this directory now lives in
`test/cpp/suites/*.cpp`, locking mapping normalization, exported ordering, fallback selection, and
provider/store-driven active-binding refresh for `HierarchyControllerProvider` and `SidebarHierarchyController`.
