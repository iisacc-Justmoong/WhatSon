# `src/app/models/sidebar`

## Role
This directory contains the controller layer that coordinates which hierarchy domain is currently active in the sidebar.

It sits above concrete hierarchy domains but below the visual sidebar QML. Its job is routing and normalization, not persistence-heavy mutation.

## Important Types
- `SidebarHierarchyController`: exposes the active hierarchy index and resolves the active hierarchy and note-list models.
- `SidebarHierarchyInteractionController`: owns chevron expansion state preservation and expansion activation suppression
  for the visual hierarchy. It also owns right-chevron stable-key derivation for hit-tested items, while
  `SidebarHierarchyView.qml` keeps only geometry hit-testing and post-commit presentation sync. Footer click dispatch and
  other view-local behavior are owned directly by `SidebarHierarchyView.qml`; the controller does not expose footer
  dispatch compatibility helpers.
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
`SidebarHierarchyInteractionController` is covered there as well, so expansion preservation, key derivation, rollback,
and activation suppression remain C++ model/controller behavior while footer action routing stays in sidebar QML.

<a id="한국어"></a>

## Korean

This section is a bottom summary for checking the above  README  content in Korean.

- Target:  ``src/app/models/sidebar``  ( `docs/src/app/models/sidebar/README.md` )
- Location:  `docs/src/app/models/sidebar`
- Role: This file describes the structure, responsibility, operating rules, and validation criteria of the corresponding directory or module.
- Current responsibility: Hierarchy expansion preservation and right-chevron stable-key derivation are owned by `SidebarHierarchyInteractionController` C++ object. Actions that end within the view, like footer action dispatch, are directly owned by `SidebarHierarchyView.qml`.
- Criteria: File path, command,  API  name, and detailed change history are maintained based on the original English text.
- On Change: If the above English text is modified, this Korean lower section is also updated to the latest state.
