# `src/app/models/detailPanel/DetailPanelCurrentHierarchyBinder.hpp`

## Role
`DetailPanelCurrentHierarchyBinder` is the composition-root coordinator that binds the active sidebar hierarchy context
into `DetailPanelController`.

## Responsibilities
- Observes `IActiveHierarchyContextSource`.
- Pushes the current note-list model and current hierarchy directory resolver into `DetailPanelController`.
- Clears those bindings cleanly when the active hierarchy context disappears.

## Dependency Direction
- Depends on the sidebar-side abstraction `IActiveHierarchyContextSource`, not on `SidebarHierarchyController`.
- Keeps `main.cpp` free of ad-hoc lambda wiring for detail-panel context synchronization.
