# `src/app/qml/view/content/editor/ContentsGutterLayer.qml`

## Responsibility

`ContentsGutterLayer.qml` renders the editor gutter chrome only: line numbers and normalized gutter markers.

The component is intentionally dumb. It does not query the editor directly. The parent view computes visible line
entries and marker geometry, then hands those values to the gutter as plain model data or narrow numeric resolvers.

## Public Surface

- `visibleLineNumbersModel`: array of `{ lineNumber, y }` entries already culled to the current viewport.
- `effectiveGutterMarkers`: normalized marker payloads (`type`, `startLine`, `lineSpan`, `color`).
- `lineNumberColumnLeft` / `lineNumberColumnTextWidth`: text column geometry.
- `markerHeightResolver` / `markerYResolver`: optional marker geometry callbacks.
- `currentCursorLineNumber`: active line used for highlight color and font weight.

## Binding Rules

- Line-number delegates no longer call `lineY(...)` on every binding evaluation. They consume a precomputed
  `resolvedY` from `visibleLineNumbersModel`.
- The parent snapshot function must hand this component a real array. If `visibleLineNumbersModel` is assigned from a
  helper that forgot to `return visibleLines`, the gutter layer intentionally renders nothing rather than guessing.
- Marker delegates still accept resolver callbacks, but all callback results pass through
  `resolveNumericResolverValue(...)` before use so invalid/undefined values collapse to safe numeric fallbacks.
- Delegate payload is always accessed through `required property var modelData`, avoiding implicit delegate context
  reads.

## Collaborators

- `ContentsDisplayView.qml`: desktop owner that computes the visible line-entry snapshot and normalized marker list.
- `MobileContentsDisplayView.qml`: mobile owner that keeps gutter visibility disabled entirely.
- `ContentsDisplayView.qml` resolves marker colors for `current`, `changed`, and `conflict`, and unknown marker types
  now fall back to the primary/current color instead of leaving the resolver path incomplete.
- `ContentsGutterMarkerBridge`: prepares external marker payloads before the parent view hands them to the gutter.

## Regression Checks

- Line numbers should remain vertically stable while scrolling rich text with wrapped lines.
- Line numbers should not disappear after editor refactors that touch the parent snapshot helpers; the gutter expects a
  concrete `visibleLineNumbersModel` array at all times.
- Active line styling should track the current cursor line without creating repeated binding-loop warnings.
- Marker pills should remain aligned with the same logical lines as the line-number snapshot.
- Marker color resolution should remain total for `current`, `changed`, `conflict`, and unexpected fallback input.
- Mobile editor routes should not instantiate visible gutter chrome or reserve gutter width.
