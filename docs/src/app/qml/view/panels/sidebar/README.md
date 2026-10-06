# `src/app/qml/view/panels/sidebar`

## Role
This directory contains the visual sidebar hierarchy surface and the domain-specific wrappers that mount it.

The directory is intentionally split between:
- one reusable visual host: `SidebarHierarchyView.qml`
- inline helper controllers for selection, rename, note-drop, and bookmark visuals inside `SidebarHierarchyView.qml`
- lightweight wrappers such as `HierarchyViewLibrary.qml`, `HierarchyViewProjects.qml`, and related domain panels

## Important Design Choice
`SidebarHierarchyView.qml` is not expected to keep every interaction detail in the root object. Recent cleanup removed
the old sibling helper files and keeps the controller-style responsibilities as named inline `QtObject` helpers:
- `hierarchySelectionController`
- `renameController`
- `noteDropController`
- `bookmarkPaletteController`

This keeps the root sidebar view focused on composition, geometry, and signal forwarding.

The rename path is intentionally split between `SidebarHierarchyView.qml` and
`renameController`: the controller owns the transaction, while the view owns the rendered
`displayedHierarchyModel` snapshot so inline rename can hide only the visible label without dropping stable row
identity.

Each inline helper must initialize its required root dependencies explicitly. If those bindings are missing, QML can
abort workspace route construction with required-property errors and show only the empty LVRS window background.

does not just rely on LVRS defaults anymore; it pushes the shared `LV.Hierarchy` surface onto a

## Relationship To C++
This directory talks to C++ almost entirely through:
- `IHierarchyController`-compatible objects
- `HierarchyInteractionBridge`
- `HierarchyDragDropBridge`

That separation is what keeps domain-specific mutation logic out of the QML file itself.

<a id="한국어"></a>

## Korean

This section is a bottom summary for checking the above  README  content in Korean.

- Target:  ``src/app/qml/view/panels/sidebar``  ( `docs/src/app/qml/view/panels/sidebar/README.md` )
- Location:  `docs/src/app/qml/view/panels/sidebar`
- Role: This file describes the structure, responsibility, operating rules, and validation criteria of the corresponding directory or module.
- Criteria: File path, command,  API  name, and detailed change history are maintained based on the original English text.
- On Change: If the above English text is modified, this Korean lower section is also updated to the latest state.
- Current baseline: The inline `QtObject` helper inside `SidebarHierarchyView.qml` owns selection, renaming, dropping, and bookmark-palette responsibilities in place of the deleted sibling helper QML file.
