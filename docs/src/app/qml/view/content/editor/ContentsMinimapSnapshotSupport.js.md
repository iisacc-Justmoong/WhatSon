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

## Notes

- The helper is intentionally text-only.
  It does not call `positionToRectangle(...)` itself; the owning editor QML files still own geometry sampling.
- This keeps the shared contract pure and lets each view decide when a full rebuild is required because of layout
  changes instead of text changes.

## Tests

- Automated test files are not currently present in this repository.
- Regression checklist:
  - A single-line edit must return a changed-line range confined to that logical line.
  - Inserting or deleting a newline must widen the affected range so the following logical-line numbering can shift.
  - Splicing replacement groups must preserve untouched prefix groups and shift suffix `lineNumber` / `contentY`
    values by the exact replacement delta.
