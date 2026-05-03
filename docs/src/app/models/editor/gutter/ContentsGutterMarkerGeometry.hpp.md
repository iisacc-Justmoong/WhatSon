# `src/app/models/editor/gutter/ContentsGutterMarkerGeometry.hpp`

## Role

Declares the C++ calculation object that turns live editor state into gutter marker entries.

## Contract

- Inputs are primitive view/model facts: mount state, current RAW `.wsnbody` text, saved RAW `.wsnbody` text,
  cursor position, line-number y entries, line-number base offset, and marker height.
- Publishes `markerEntries`, a list of maps consumed by `contents/Gutter.qml`.
- Publishes `cursorLineNumber` so the gutter can color the active line number from the same cursor calculation.
- Uses marker `type` values:
  - `cursor`: current `LV.TextEditor` cursor line, rendered as the blue marker.
  - `unsaved`: current source lines that differ from the saved `.wsnbody` snapshot, rendered as yellow markers.

## Boundary

The object does not read files and does not own persistence. It compares the current editor RAW text with the saved RAW
snapshot supplied by the runtime view and projects the result onto the already calculated line-number coordinate space.
