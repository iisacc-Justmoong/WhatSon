# `src/app/qml/view/content/editor/ContentsCalloutLayer.qml`

## Responsibility

`ContentsCalloutLayer.qml` renders proprietary `<callout>` source blocks as a Figma-aligned callout row UI.

The layer is backend-fed:
- input: canonical source text (`sourceText`)
- parser backend: `calloutBackend.parseCallouts(sourceText)` (`ContentsCalloutBackend`)
- output: LVRS callout rows (`background + 1px divider + text`)

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

## Regression Checks

- A source `<callout>One</callout><callout>Two</callout>` must render two callout rows in source order.
- A multi-line source `<callout>Line 1\nLine 2</callout>` must render as one row with expanded background/divider
  height.
- Empty or unsupported source must produce zero callout rows.
