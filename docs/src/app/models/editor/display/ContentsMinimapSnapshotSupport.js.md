# `src/app/models/editor/display/ContentsMinimapSnapshotSupport.js`

## Responsibility

Provides the shared minimap row-flattening helper used by both desktop and mobile contents editors.

## Public Helpers

- `flattenLineGroups(lineGroups, fallbackLineHeight)`
  Converts logical-line groups back into the flat minimap row list consumed by `ContentsMinimapLayer.qml`.
  Structured block groups may also carry row-style hints such as `contentWidth`, `contentAvailableWidth`,
  `visualRowWidths`, `minimapRowCharCount`, and `minimapVisualKind`, letting the same flattening pass preserve either
  parser-derived block silhouettes or measured plain-text row widths without owning those policies itself.

## Notes

- The helper is intentionally paint-model only.
  It does not call `positionToRectangle(...)` itself; the owning editor QML files and coordinators still decide
  whether a given note uses parser-derived block rows or measured text geometry.
- The helper still does not know editor-specific block types directly; it only honors the optional visual-hint fields
  already attached by the owning QML host.
- It no longer owns changed-line range detection or cached group splicing. The editor display host rebuilds minimap
  groups from the current document state and uses this helper only to flatten those groups for painting.

## Tests

- Regression coverage lives in
  `test/cpp/suites/contents_display_minimap_coordinator_tests.cpp`.
- The maintained checks cover:
  - preservation of explicit `visualRowWidths` during flattening.
