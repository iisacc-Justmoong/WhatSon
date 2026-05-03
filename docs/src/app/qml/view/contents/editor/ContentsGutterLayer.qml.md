# `src/app/qml/view/contents/editor/ContentsGutterLayer.qml`

## Responsibility

`ContentsGutterLayer.qml` renders the editor gutter chrome only: line numbers and normalized gutter markers.

The component is intentionally dumb. It does not query the editor directly. The parent view computes visible line
entries and marker geometry, then hands those values to the gutter as plain model data or narrow numeric resolvers.

## Public Surface

- `visibleLineNumbersModel`: array of `{ lineNumber, y }` entries already culled to the current viewport.
- `effectiveGutterMarkers`: normalized external marker payloads (`type`, `startLine`, `lineSpan`, `color`).
  The parent no longer injects an implicit cursor/current-line marker into this model.
- `lineNumberColumnLeft` / `lineNumberColumnTextWidth`: text column geometry.
- `lineNumberRightInset`: dedicated breathing room between the right-aligned line numbers and the note body.
  The parent now keeps this smaller than the editor text inset so the gutter no longer feels visually detached.
- `markerHeightResolver` / `markerYResolver`: optional marker geometry callbacks.
- `currentCursorLineNumber`: active line used for highlight color and font weight.

## Binding Rules

- Line-number delegates no longer call `lineY(...)` on every binding evaluation. They consume a precomputed
  `resolvedY` from `visibleLineNumbersModel`.
- That `resolvedY` is now gutter-specific rather than plain editor-line Y.
  The host pre-applies wrapped-row compensation, so each logical line number is pushed down by the extra visual-row
  height created by prior soft wraps.
- Rail offsets, marker width, line-number font size, and active/default gutter colors now come from LVRS gap/theme
  tokens plus `LV.Theme.scaleMetric(...)`, so gutter chrome scales with the same density policy as the host editor.
- The parent snapshot function must hand this component a real array. If `visibleLineNumbersModel` is assigned from a
  helper that forgot to `return visibleLines`, the gutter layer intentionally renders nothing rather than guessing.
- The parent-side gutter coordinators must also preserve populated line arrays while viewport height is still pending.
  A zero-height first layout pass is not allowed to degrade the model back to the one-line fallback when the note body
  already has multiple logical lines.
- Marker delegates still accept resolver callbacks, but all callback results pass through
  `resolveNumericResolverValue(...)` before use so invalid/undefined values collapse to safe numeric fallbacks.
- Delegate payload is always accessed through `required property var modelData`, avoiding implicit delegate context
  reads.

## Collaborators

- `ContentsDisplayView.qml`: unified owner that computes the visible line-entry snapshot and normalized marker list.
  In mobile mode, `ContentsDisplayHostModePolicy.qml` keeps gutter visibility disabled entirely.
- The shared host now hard-clamps the gutter layout width to the resolved gutter token, so host relayout pressure
  cannot squeeze the gutter narrower than its intended chrome width.
- `ContentsDisplayView.qml` resolves marker colors for `current`, `changed`, and `conflict`, and unknown marker types
  now fall back to the primary/current color instead of leaving the resolver path incomplete.
- `ContentsGutterMarkerBridge`: prepares external marker payloads before the parent view hands them to the gutter.

## Regression Checks

- Line numbers should remain vertically stable while scrolling rich text with wrapped lines.
- Soft-wrapped logical lines must push every following gutter label down by the same accumulated wrapped-row height.
- Markdown-style list typing must not narrow or widen the allocated gutter strip while the editor surface relayouts.
- Line numbers should not disappear after editor refactors that touch the parent snapshot helpers; the gutter expects a
  concrete `visibleLineNumbersModel` array at all times.
- Active line styling should track the current cursor line without creating repeated binding-loop warnings.
- Active line styling should not create a separate current-line marker dot; the active line is indicated by the line
  number text style only.
- Marker pills should remain aligned with the same logical lines as the line-number snapshot.
- Marker color resolution should remain total for `current`, `changed`, `conflict`, and unexpected fallback input.
- Mobile editor routes should not instantiate visible gutter chrome or reserve gutter width.
