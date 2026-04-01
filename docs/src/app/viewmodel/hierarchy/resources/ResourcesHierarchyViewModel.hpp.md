# `src/app/viewmodel/hierarchy/resources/ResourcesHierarchyViewModel.hpp`

## Responsibility

This header declares the read-mostly resources hierarchy viewmodel. It presents resources as a
metadata-driven LVRS hierarchy while tracking the current resource-path payload loaded from
`Resources.wsresources`.

## Public Contract

- Publishes the usual hierarchy row model, selection, count, and load-state properties.
- Implements `IHierarchyExpansionCapability` so open/closed state is owned by the viewmodel.
- Exposes `setResourcePaths(...)` for direct input and `applyRuntimeSnapshot(...)` for runtime
  snapshot loads.
- Exposes `requestViewModelHook()` as a file-backed refresh hook that re-reads
  `Resources.wsresources` (and fallback package scans) when the source path is known.
- Still exposes rename/create/delete entry points because it conforms to the shared hierarchy
  surface, but resources remains functionally read-only.
- Declares the inherited rename/crud/expansion entry points with explicit `override` markers so the
  shared hierarchy contract remains warning-clean.

## Refresh Rules

- Runtime updates may change type/format rows according to current package metadata.
- Expansion state must survive `setResourcePaths(...)`, `applyRuntimeSnapshot(...)`, and
  `requestViewModelHook()` reloads.

## Internal State

- `m_resourcePaths` stores the latest parsed file list.
- `m_items` stores the current materialized hierarchy rows plus UI expansion state.
- `m_resourcesFilePath` identifies the source `Resources.wsresources` file.
