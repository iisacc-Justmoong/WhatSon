# `src/app/viewmodel/hierarchy/resources/ResourcesHierarchyViewModelSupport.hpp`

## Responsibility

This header owns resource-specific metadata materialization and the flattened `type -> format` hierarchy projection used by the resources sidebar.

## Shared IO Delegation

`ResourcesSupport` now re-exports shared hub IO from `WhatSonHierarchyIoSupport.hpp`:

- `normalizePath(...)`
- `resolveContentsDirectories(...)`
- `readUtf8File(...)`
- `deduplicateStringsPreservingOrder(...)`

This removes another copy of the `.wshub` traversal and UTF-8 file loading logic from the resources domain.

## Domain Logic That Stays Local

The resources support header still owns the logic that is unique to resource packages:

- resource path sanitization
- metadata parsing and fallback materialization
- conversion from materialized resources to stable hierarchy rows
- equality helpers for no-op reset detection

`sanitizeStringList(...)` and `extractResourcePathsFromItems(...)` now use `QSet`-backed duplicate
tracking so large imported resource lists avoid repeated `QStringList::contains(...)` scans.

## Materialization Rule

Each `resourcePath` is resolved through `WhatSonResourcePackageSupport.hpp` first.

- If package metadata is valid, it is used directly.
- Otherwise, fallback metadata is synthesized from the legacy raw path.

This stage normalizes `resourceId`, `bucket`, `type`, `format`, and `assetPath`.

## Stable Keys

Rows keep explicit keys so expansion state can be restored safely.
Typical examples:

- `type:image`
- `format:image:.png`

These keys are the persistence anchor for `ResourcesHierarchyViewModel` expansion-restore logic.
Format keys use a normalized lookup form, so `.PNG` and `.png` collapse into the same format node.

## Empty-State Fallback

`buildHierarchyItems(...)` always renders a type-parent tree and attaches a default format catalog
to each type. Imported resource metadata extends this catalog with extra formats, but the baseline
format list is visible even when `resourcePaths` is empty.

- `Image`
- `Video`
- `Document`
- `3D Model`
- `Web page`
- `Music`
- `Audio`
- `ZIP`
- `Other`

These type rows are expandable (`kind="type"`) and each expands into format children (`kind="format"`),
so the sidebar keeps the legacy `type parent -> format children` interaction from the old hierarchy UI.

## Equality Contract

`hierarchyItemsEqual(...)` compares every structural field used by QML rendering:

- depth and label
- expansion and chevron flags
- key, kind, bucket, type, and format
- resource identity and resolved paths

`ResourcesHierarchyViewModel::setResourcePaths(...)` uses this comparator to skip no-op model resets
when the rebuilt hierarchy is identical to the previous one.

## Maintenance Rule

Keep shared filesystem behavior in `WhatSon::Hierarchy::IoSupport`.
Only resource-domain parsing and hierarchy shaping should live in this header.
