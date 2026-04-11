# `src/app/qml/view/content/editor/ContentsCalloutLayer.qml`

## Responsibility

`ContentsCalloutLayer.qml` renders proprietary `<callout>` source blocks as a Figma-aligned callout row UI.

The layer is renderer-fed:
- input: renderer-owned callout models (`renderedCallouts`)
- output: LVRS callout rows (`background + 1px divider + text`)
- focus hook: `blockFocusHandler(focusSourceOffset)`
- placement hook: `sourceOffsetYResolver(sourceStart)` (host-provided source-offset -> viewport-y resolver)

## Rendering Contract

- Root row style follows Figma node `280:7897`:
  - background `#262728`
  - row padding `4`
  - inner spacing `12`
- Left divider:
  - width `1`
  - height `max(14, text height)`
  - color `#D9D9D9`
- Body text:
  - color `#FFFFFF`
  - `Pretendard` / `12px` / `Medium`
  - multi-line wrapping (`Wrap`)

## Render-Model Rules

- Source parsing rules are now owned by `src/app/editor/renderer/ContentsStructuredBlockRenderer.cpp`.
- This QML layer consumes only renderer-provided callout entries and does not parse RAW source locally.
- Parse entries include `sourceStart` and `focusSourceOffset`; rows are positioned by source location and can restore
  editor focus to callout-body start.
- The host-provided `sourceOffsetYResolver(...)` now resolves editor-internal document Y, so callout rows stay aligned
  with the live editor content coordinate space.
- The layer normalizes `QVariantList`-like values into JS arrays so C++ renderer properties still produce visible
  callout rows in QML.

## Regression Checks

- A source `<callout>One</callout><callout>Two</callout>` must render two callout rows in source order.
- A multi-line source `<callout>Line 1\nLine 2</callout>` must render as one row with expanded background/divider
  height.
- An empty source without `<callout>` tags must produce zero callout rows, but an empty `<callout></callout>` tag must
  still produce one visible callout row.
- Callout rows must be placed at resolved source locations (`sourceOffsetYResolver`) so authored tag position is
  reflected on editor surface.
- Tapping a callout row must route focus back to callout-body start in RAW source.
