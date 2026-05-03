# `src/app/qml/view/contents/Gutter.qml`

## Role
`Gutter.qml` renders the Figma `ContentsView` gutter rail for node `155:5345`.

## Contract
- Imports LVRS and uses LVRS color, metric, radius, and typography tokens.
- Exposes `lineNumberCount`, `activeLineNumber`, and semantic marker color properties for the root contents frame.
- Exposes resolved `lineNumberColumnLeft` and `lineNumberColumnTextWidth` properties so the runtime editor shell can
  reuse this Figma gutter without leaving fallback arithmetic in QML.
- Consumes `lineNumberEntries` from `ContentsGutterLineNumberGeometry`; each entry supplies the displayed source-line
  number plus its gutter-coordinate y position and resolved source-line height.
- Consumes `markerEntries` from `ContentsGutterMarkerGeometry`; each entry supplies a semantic marker type plus a
  gutter-coordinate y/height pair.
- Emits `viewHookRequested(string reason)` through `requestViewHook(reason)` for the standard view hook surface.
- Does not expose or paint a gutter background color. The gutter visually merges with the editor surface.

## UI
- The token-composed host width is assigned by `ContentsView.qml`.
- The file owns right-aligned line labels, blue cursor marker rendering, and yellow unsaved line marker rendering; it
  does not introduce a visible background behind those markers.
- Gutter/minimap calculation policy lives in `src/app/models/editor/gutter` and
  `src/app/models/editor/minimap`; this file only renders the resolved metrics.
- Line labels are absolutely positioned from resolved entries. They must not be stacked with `Column`, because the
  gutter must match the live editor source-line geometry.
- Line labels use each entry's `height` when available. A resource row becomes taller only when the resolved editor
  geometry places the next source line below the rendered frame; `Gutter.qml` does not infer that height itself.
- Default line-number and marker inputs are token-only placeholders. Real design/runtime values are supplied by the
  host from `ContentsGutterLayoutMetrics`, `ContentsGutterLineNumberGeometry`, and `ContentsGutterMarkerGeometry`.
