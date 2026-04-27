# `ContentsGutterMarkerBridge.cpp`

- Normalizes externally supplied gutter marker payloads before `ContentsDisplayView.qml` paints the gutter layer.
- Only durable document markers such as `changed` and `conflict` are accepted.
- Transient `current` markers are intentionally ignored; the current cursor line is represented by line-number color
  and font weight only, so the editor surface does not draw a stray dot beside paragraph text.
