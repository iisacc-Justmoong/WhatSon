# `src/app/viewmodel/panel/HierarchyInteractionBridge.cpp`

## Role
This implementation binds a live hierarchy viewmodel to the interaction bridge and keeps a cached capability state for QML.

## Core Behavior
- `setHierarchyViewModel(...)` verifies the `View -> ViewModel` dependency edge through `verifyDependencyAllowed(...)`.
- The bridge stores the cast `IHierarchyViewModel*` as a `QPointer`.
- It listens to `hierarchySelectionChanged` and `hierarchyNodesChanged`.
- On every relevant change it recalculates the cached capability booleans.

## Capability Resolution
Capability checks are done through `qobject_cast`:
- rename support comes from `IHierarchyRenameCapability`
- create/delete/view-options support comes from `IHierarchyCrudCapability`
- expansion support comes from `IHierarchyExpansionCapability`

The cached property approach matters because QML can bind to stable boolean properties instead of repeatedly probing capabilities from JavaScript.

## Bulk Expansion Path

- `setAllItemsExpanded(...)` first tries a direct meta-object call to a concrete viewmodel `Q_INVOKABLE`.
- If that bulk entry point is unavailable, the bridge falls back to the per-row `IHierarchyExpansionCapability`
  contract and iterates only rows whose serialized hierarchy node exposes `showChevron: true`.
- This keeps the QML footer menu small while still allowing individual domains to provide an optimized bulk path later.

## Failure Model
If the viewmodel is missing, does not implement a capability, or the capability is disabled, the bridge fails closed.
- boolean queries return `false`
- commands become no-ops

This makes the QML side simpler and avoids partial mutation behavior when a domain does not support a feature.
