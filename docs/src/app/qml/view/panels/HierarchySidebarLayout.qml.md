# `src/app/qml/view/panels/HierarchySidebarLayout.qml`

## Role
This component is the adapter between the generic sidebar view and the runtime hierarchy routing layer.

It does three important jobs.
- Resolve the currently active hierarchy domain.
- Bind that domain to a stable LVRS view ID.
- Instantiate the two bridge objects used by the visual sidebar.

## Binding Strategy
The component defines `hierarchyViewId` as `HierarchySidebarLayout.activeHierarchy`.

It then maps the current sidebar hierarchy index to a root registry key such as:
- `libraryHierarchyViewModel`
- `projectsHierarchyViewModel`
- `bookmarksHierarchyViewModel`
- `tagsHierarchyViewModel`

`syncHierarchyViewBinding()` binds that key into `LV.ViewModels` with write ownership. This means the sidebar can always talk to the active hierarchy through a stable per-view binding even though the actual concrete domain changes.

## Hosted Bridges
- `HierarchyDragDropBridge`: reorder and note-drop bridge.
- `HierarchyInteractionBridge`: rename, create, delete, and expansion bridge.

## Why This File Is Important
This is the QML seam where the repository's "one hierarchy type, one dedicated viewmodel" rule is translated into a visual sidebar that can switch domains at runtime.
