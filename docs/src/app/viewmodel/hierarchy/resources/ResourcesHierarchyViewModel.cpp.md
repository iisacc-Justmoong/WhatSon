# `src/app/viewmodel/hierarchy/resources/ResourcesHierarchyViewModel.cpp`

## Responsibility

This implementation exposes the resources sidebar as a metadata-driven hierarchy built from
`.wsresource` packages rather than as a hardcoded static tree.

## Tree Contract

`setResourcePaths(...)` does not only store path strings. It materializes each resource reference
into a two-level hierarchy:

- `type`
- `format`

When the input path list is empty, the viewmodel now publishes default top-level type rows
(`Image`, `Video`, `Document`, `3D Model`, `Web page`, `Music`, `Audio`, `ZIP`, `Other`) so the
resources sidebar does not render as a fully empty panel.

## Expansion Preservation

During rebuild, the previous `key -> expanded` state is restored so runtime snapshot updates do not
collapse already opened type/format rows.

## Load Fallback

`loadFromWshub(...)` first parses `.wscontents/Resources.wsresources`. When that file is missing or
contains no paths, it falls back to scanning the hub-level `*.wsresources` directory for
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

## Count Role Compatibility

`depthItems()` includes a numeric `count` field on every row. The resources domain is not the owner
of note-index membership, so it emits `0` to preserve shared `LV.Hierarchy` payload compatibility.
