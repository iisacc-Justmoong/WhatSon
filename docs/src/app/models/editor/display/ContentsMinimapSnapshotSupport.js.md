# `src/app/models/editor/display/ContentsMinimapSnapshotSupport.js`

## Responsibility

Provides the shared incremental minimap snapshot helpers used by both desktop and mobile contents editors.

## Public Helpers

- `computeChangedLineRange(previousText, nextText)`
  Computes the logical-line window affected by a text edit using a prefix/suffix diff.
- `spliceLineGroups(existingGroups, replacementGroups, previousStartLine, previousEndLine)`
  Replaces the cached minimap line-group slice for the edited range and shifts trailing line numbers / `contentY`
  offsets without forcing a full rebuild.
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
- Incremental splices preserve measured minimap width metadata so a changed middle line does not collapse neighboring
  rows back to character-count-derived bar widths.

## Tests

- Regression coverage lives in
  `test/cpp/suites/contents_display_minimap_coordinator_tests.cpp`.
- The maintained checks cover:
  - changed-line range confinement for ordinary text edits,
  - suffix/prefix splice stability for cached minimap groups,
  - preservation of explicit `visualRowWidths` during flattening.
