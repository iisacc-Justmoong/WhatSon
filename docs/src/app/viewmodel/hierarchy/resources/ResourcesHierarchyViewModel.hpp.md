# `src/app/viewmodel/hierarchy/resources/ResourcesHierarchyViewModel.hpp`

## Responsibility

This header declares the read-mostly resources hierarchy viewmodel. It presents the supported
resource categories as a stable LVRS tree while tracking the current resource-path payload loaded
from `Resources.wsresources`.

## Public Contract

- Publishes the usual hierarchy row model, selection, count, and load-state properties.
- Implements `IHierarchyExpansionCapability` so open/closed state is owned by the viewmodel.
- Exposes `setResourcePaths(...)` for direct input and `applyRuntimeSnapshot(...)` for runtime
  snapshot loads.
- Still exposes rename/create/delete entry points because it conforms to the shared hierarchy
  surface, but resources remains functionally read-only.
- Declares the inherited rename/crud/expansion entry points with explicit `override` markers so the
  shared hierarchy contract remains warning-clean.

## Refresh Rules

- The supported type taxonomy is structurally constant.
- Runtime updates are therefore allowed to replace `m_resourcePaths` without rebuilding `m_items`
  once the static tree has been initialized.
- Expansion state must survive `setResourcePaths(...)` and `applyRuntimeSnapshot(...)`.

## Internal State

- `m_resourcePaths` stores the latest parsed file list.
- `m_items` stores the constant supported-type tree plus UI expansion state.
- `m_resourcesFilePath` identifies the source `Resources.wsresources` file.
