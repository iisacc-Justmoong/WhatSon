# `src/app/qml/view/panels/HierarchySidebarLayout.qml`

## Role
This component is the adapter between the generic sidebar view and the runtime hierarchy routing layer.

It does four important jobs.
- Resolve the currently active hierarchy domain.
- Select the matching domain controller from `SidebarHierarchyController`.
- Instantiate the two bridge objects used by the visual sidebar.
- Instantiate the sidebar interaction policy controller that keeps footer and expansion decisions in C++.

## Binding Strategy
`resolvedHierarchyController` resolves from
`sidebarHierarchyController.hierarchyControllerForIndex(currentHierarchy)` first, then falls back through
`sidebarHierarchyController.resolvedHierarchyController`.
This keeps mobile and desktop aligned to the same active domain object even when the active hierarchy index changes
slightly earlier than one previously-resolved provider object.

## Hosted Bridges
- `HierarchyDragDropBridge`: reorder and note-drop bridge.
- `HierarchyInteractionBridge`: rename, create, delete, and expansion bridge.
- `SidebarHierarchyInteractionController`: footer dispatch, duplicate trigger coalescing, and expansion-state policy
  controller bound to the active hierarchy index.

## Layout Notes
- The shared toolbar frame width now resolves through `LV.Theme.inputMinWidth + LV.Theme.gap20` instead of a raw
  `200px` constant, keeping the sidebar toolbar track aligned with LVRS density scaling on desktop and mobile.

## Why This File Is Important
This is the QML seam where the repository's "one hierarchy type, one dedicated controller" rule is translated into a visual sidebar that can switch domains at runtime.
