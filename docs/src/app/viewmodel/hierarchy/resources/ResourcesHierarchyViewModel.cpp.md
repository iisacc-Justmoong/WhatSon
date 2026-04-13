# `src/app/viewmodel/hierarchy/resources/ResourcesHierarchyViewModel.cpp`

## Responsibility

This implementation exposes the resources sidebar as a metadata-driven hierarchy built from
`.wsresource` packages rather than as a hardcoded static tree.

## Tree Contract

`setResourcePaths(...)` does not only store path strings. It materializes each resource reference
into a two-level hierarchy:

- `type`
- `format`

The hierarchy keeps the legacy interaction contract:

- top-level rows are resource types
- expanding a type reveals format rows

Even when the input path list is empty, the viewmodel still publishes the type parents with their
default format catalog, so the resources sidebar is never a flat type-only list.

## Right-Panel List Contract

This viewmodel now owns a dedicated `ResourcesListModel` projection for the shared right panel.

- `selectedIndex = -1`: no hierarchy row is selected yet, so the right-panel list now projects the full resource set.
- selected `kind="type"` row: only resources of that type are projected.
- selected `kind="format"` row: only resources that match type+format are projected.

Each projected row carries:

- `noteId`: resolved `.wsresource` package directory path (absolute when possible)
- `primaryText`: resolved asset display name
- `bodyText`: normalized `<resource ...>` tag text used by the editor surface
- `image/imageSource`: image resources only, so list cards can show inline thumbnails
- `type/format/resourcePath/resolvedPath/source/renderMode/displayName`: resource viewer metadata
  used by renderer and list delegates

This is the bridge that fixes the previous `No list data` behavior in the resources domain, including the
hierarchy-toolbar transition where the resources list must appear immediately even before the user picks a
type/format row.

## Resource Deletion Contract

`deleteNoteById(...)` / `deleteNotesByIds(...)` now treat right-panel list ids as resource-package
targets.

- Each id is resolved back to the owning `.wsresource` directory.
- The matching `Resources.wsresources` entry is removed before the package directory is deleted.
- If directory removal fails, the hierarchy file write is rolled back to the previous resource-path
  list.
- Successful deletions emit `hubFilesystemMutated()` so hub sync continues to mark the edit as a
  local mutation.

This is the path used by `ListBarLayout` when the active note list belongs to the resources domain,
including `Delete` / `Backspace` keyboard deletion.

## Idempotent Update Guard

`setResourcePaths(...)` is idempotent. After sanitizing paths and rebuilding hierarchy rows, it
returns early when both are unchanged:

- no `syncModel()` reset
- no `hierarchyModelChanged` signal
- no forced `setSelectedIndex(-1)`

This guard prevents hook-driven reload loops in `SidebarHierarchyView` when the resources payload is
stable (for example repeated `rawCount=0` reloads).

## Expansion Preservation

During rebuild, the previous `key -> expanded` state is restored so runtime snapshot updates do not
collapse already opened type/format rows.

`setResourcePaths(...)` also preserves the currently selected hierarchy row by `key` when that row
still exists after a resource-path rebuild. This keeps the right-panel resource list alive after
package deletion or import instead of dropping back to a blank transition state.

## Regression Checks

- Switching the active hierarchy to `Resources` must not leave the shared right-panel list blank while no
  type/format node is selected yet.
- Deleting or importing `.wsresource` packages must preserve the current projection when the selected taxonomy
  key still exists after rebuild.

## Load Fallback

`loadFromWshub(...)` first parses `.wscontents/Resources.wsresources`. When that file is missing or
contains no paths, it falls back to scanning all hub-level resource roots (`.wsresources` and
`*.wsresources`) for
`.wsresource` packages.

The canonical payload for this domain therefore remains a normalized list of `.wsresource` package
paths.

## ViewModel Hook Contract

`requestViewModelHook()` performs file-backed reload when `m_resourcesFilePath` is known.

- `reloadFromResourcesFilePath(...)` reparses `Resources.wsresources` when present.
- If the parsed path list is empty, it re-applies the package scan fallback so the hierarchy still
  reflects on-disk resources.
- It recomputes resource reference base paths (including the resolved `.wshub` root) before
  rebuilding rows, keeping preview/asset resolution stable after hook refresh.
- Base-path changes also trigger resource list re-materialization so right-panel paths stay
  resolved when hub layout roots change.

## Count Role Compatibility

`depthItems()` includes a numeric `count` field on every row and forwards the materialized hierarchy
count from `ResourcesHierarchyItem`.

- `type` rows expose the number of resources classified into that type.
- `format` rows expose the number of resources classified into that format.

This value is consumed directly by the right-side count renderer in `LV.Hierarchy`.
