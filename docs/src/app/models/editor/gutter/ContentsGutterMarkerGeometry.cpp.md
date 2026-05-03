# `src/app/models/editor/gutter/ContentsGutterMarkerGeometry.cpp`

## Role

Implements gutter marker projection for the live editor gutter.

## Behavior

- Cursor marker y is derived by converting `cursorPosition` to a source line index and then looking up that line in
  `lineNumberEntries`.
- Unsaved markers are derived by comparing current RAW source lines with the saved RAW `.wsnbody` snapshot.
- Consecutive unsaved lines are coalesced into one marker span so large edits do not create unnecessary QML delegates.
- Markers are emitted in visual stacking order: unsaved spans first, cursor marker last. This keeps the blue cursor
  marker visible even when the cursor is on an unsaved line.
- If live line y data is temporarily missing, the object falls back to the first known y plus `markerHeight`.

## Tests

- `contentsGutterMarkerGeometry_marksCursorAndUnsavedLineSpans`
