# `src/app/qml/view/content/editor/ContentsMinimapSnapshotSupport.js`

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
  Structured block groups may also carry row-style hints such as `minimapRowCharCount` / `minimapVisualKind`, letting
  the same flattening pass preserve image-style block silhouettes instead of always slicing one group's total text
  length across every visual row.

## Notes

- The helper is intentionally text-only.
  It does not call `positionToRectangle(...)` itself; the owning editor QML files still own geometry sampling.
- This keeps the shared contract pure and lets each view decide when a full rebuild is required because of layout
  changes instead of text changes.
- The helper still does not know editor-specific block types directly; it only honors the optional visual-hint fields
  already attached by the owning QML host.

## Tests

- Automated test files are not currently present in this repository.
- Regression checklist:
  - A single-line edit must return a changed-line range confined to that logical line.
  - Inserting or deleting a newline must widen the affected range so the following logical-line numbering can shift.
  - Splicing replacement groups must preserve untouched prefix groups and shift suffix `lineNumber` / `contentY`
    values by the exact replacement delta.
  - Structured image/resource groups that carry a fixed `minimapRowCharCount` must keep that per-row width after
    splicing/flattening instead of regressing to text-segment slicing.
