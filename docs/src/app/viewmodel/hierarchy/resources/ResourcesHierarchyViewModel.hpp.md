# `src/app/viewmodel/hierarchy/resources/ResourcesHierarchyViewModel.hpp`

## Responsibility

This header declares the read-mostly resources hierarchy viewmodel. It presents resources as a
metadata-driven LVRS hierarchy while tracking the current resource-path payload loaded from
`Resources.wsresources`.

## Public Contract

- Publishes the usual hierarchy row model, selection, count, and load-state properties.
- Publishes `noteListModel` (`ResourcesListModel`) so the resources domain can drive the shared
  `ListBarLayout` note-card surface.
- Implements `IHierarchyExpansionCapability` so open/closed state is owned by the viewmodel.
- Exposes `setResourcePaths(...)` for direct input and `applyRuntimeSnapshot(...)` for runtime
  snapshot loads.
- Exposes `noteDirectoryPathForNoteId(...)` for editor-side resource rendering: when a list item id
  points to a `.wsresource` package, the renderer can resolve that package directory directly.
- Exposes `deleteNoteById(...)` / `deleteNotesByIds(...)` so the shared `ListBarLayout` delete
  shortcuts can remove selected `.wsresource` packages from the resources domain without routing
  through the library-note mutation path.
- Exposes `requestViewModelHook()` as a file-backed refresh hook that re-reads
  `Resources.wsresources` (and fallback package scans) when the source path is known.
- Still exposes rename/create/delete entry points because it conforms to the shared hierarchy
  surface, but the hierarchy taxonomy remains read-only even though concrete resource packages are
  now deletable from the right-panel list.
- Declares the inherited rename/crud/expansion entry points with explicit `override` markers so the
  shared hierarchy contract remains warning-clean.
- Emits `hubFilesystemMutated()` after successful package deletion so hub-sync wiring can continue
  to classify the change as a local mutation.

## Refresh Rules

- Runtime updates may change type/format rows according to current package metadata.
- Expansion state must survive `setResourcePaths(...)`, `applyRuntimeSnapshot(...)`, and
  `requestViewModelHook()` reloads.

## Internal State

- `m_resourcePaths` stores the latest parsed file list.
- `m_items` stores the current materialized hierarchy rows plus UI expansion state.
- `m_noteListModel` stores right-panel resource cards filtered by current hierarchy selection.
- `m_resourcesFilePath` identifies the source `Resources.wsresources` file.
