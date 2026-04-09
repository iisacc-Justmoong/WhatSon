# `src/app/qml/view/panels/sidebar`

## Role
This directory contains the visual sidebar hierarchy surface and the domain-specific wrappers that mount it.

The directory is intentionally split between:
- one reusable visual host: `SidebarHierarchyView.qml`
- helper controllers for rename, note-drop, and bookmark visuals
- lightweight wrappers such as `HierarchyViewLibrary.qml`, `HierarchyViewProjects.qml`, and related domain panels

## Important Design Choice
`SidebarHierarchyView.qml` is no longer expected to contain every interaction detail inline. Recent refactoring moved several controller-style responsibilities into sibling helper files:
- `SidebarHierarchyRenameController.qml`
- `SidebarHierarchyNoteDropController.qml`
- `SidebarHierarchyBookmarkPaletteController.qml`

This keeps the root sidebar view focused on composition, geometry, and signal forwarding.

The rename path is intentionally split between `SidebarHierarchyView.qml` and
`SidebarHierarchyRenameController.qml`: the controller owns the transaction, while the view owns the rendered
`displayedHierarchyModel` snapshot so inline rename can hide only the visible label without dropping stable row
identity.

The sidebar also now owns an explicit mobile kinetic-scroll contract. `SidebarHierarchyView.qml`
does not just rely on LVRS defaults anymore; it pushes the shared `LV.Hierarchy` surface onto a
touch-friendly overshoot/flick profile so mobile hierarchy scrolling keeps momentum after release.

## Relationship To C++
This directory talks to C++ almost entirely through:
- `IHierarchyViewModel`-compatible objects
- `HierarchyInteractionBridge`
- `HierarchyDragDropBridge`

That separation is what keeps domain-specific mutation logic out of the QML file itself.
