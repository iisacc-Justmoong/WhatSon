# `src/app/models/editor/gutter/ContentsGutterLayoutMetrics.cpp`

## Role

Implements the editor gutter layout calculations declared by `ContentsGutterLayoutMetrics.hpp`.

## Calculation Rules

- Default gutter width is three `gap24` columns plus `gap2`.
- Runtime gutter width override wins only when it is greater than `gapNone`.
- Runtime line count is clamped to at least the stroke-derived minimum unit.
- Design line count remains available for the standalone Figma `view/contents/ContentsView.qml` frame.
- Line-number column position and width use explicit overrides when supplied, otherwise they fall back to token-backed
  defaults.
- Change/conflict marker heights, marker vertical offsets, icon-rail offset, and the line-number base offset are also
  resolved here so `Gutter.qml` does not retain token-composition arithmetic.
