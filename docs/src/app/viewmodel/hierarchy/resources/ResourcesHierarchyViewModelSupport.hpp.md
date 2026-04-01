# `src/app/viewmodel/hierarchy/resources/ResourcesHierarchyViewModelSupport.hpp`

## Responsibility

The resources hierarchy support header owns three responsibilities:

- `Resources.wsresources` and `.wshub` contents-path resolution
- `.wsresource` metadata materialization
- conversion from materialized entries to flattened `type -> format` hierarchy rows

## Materialization Rule

Each `resourcePath` is resolved through `WhatSonResourcePackageSupport.hpp` first.

- If package metadata is valid, it is used directly.
- Otherwise, fallback metadata is synthesized from the legacy raw path.

This stage normalizes `resourceId`, `bucket`, `type`, `format`, and `assetPath`.

## Stable Keys

Rows use explicit keys so expansion state can be restored safely.

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
so the sidebar keeps the legacy "type parent -> format children" interaction from the old hierarchy UI.

## Count Aggregation

`buildHierarchyItems(...)`는 materialized 리소스를 순회하면서 count를 같이 집계한다.

- `type.count`: 해당 타입으로 분류된 리소스 총 개수
- `format.count`: 해당 타입/포맷 조합에 속한 리소스 개수

이 값은 `ResourcesHierarchyViewModel::depthItems()`를 통해 QML count role로 전달된다.

## Equality Contract

`hierarchyItemsEqual(...)` compares every structural field used by QML rendering:

- depth/label
- expansion and chevron flags
- key/kind/bucket/type/format
- resource identity and resolved paths

`ResourcesHierarchyViewModel::setResourcePaths(...)` uses this comparator to skip no-op model resets
when the rebuilt hierarchy is identical to the previous one.
