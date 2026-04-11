# `src/app/qml/view/content/editor/ContentsCalloutLayer.qml`

## Responsibility

`ContentsCalloutLayer.qml` renders proprietary `<callout>` source blocks as a Figma-aligned callout row UI.

The layer is backend-fed:
- input: canonical source text (`sourceText`)
- parser backend: `calloutBackend.parseCallouts(sourceText)` (`ContentsCalloutBackend`)
- output: LVRS callout rows (`background + 1px divider + text`)
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

## Parsing Rules

- Source parsing rules are implemented in `src/app/callout/ContentsCalloutBackend.cpp`.
- This QML layer consumes only the backend parse result model and does not perform source-regex parsing locally.
- Parse entries now include `sourceStart`; rows are positioned by source location instead of only top stacking.
- The layer now normalizes backend `QVariantList`-like return values into JS arrays instead of depending on
  `Array.isArray(...)` only, so C++ invokable parse results still produce visible callout rows in QML.

## Regression Checks

- A source `<callout>One</callout><callout>Two</callout>` must render two callout rows in source order.
- A multi-line source `<callout>Line 1\nLine 2</callout>` must render as one row with expanded background/divider
  height.
- Empty or unsupported source must produce zero callout rows.
- Callout rows must be placed at resolved source locations (`sourceOffsetYResolver`) so authored tag position is
  reflected on editor surface.
