# `src/app/qml/view/content/editor/ContentsCalloutLayer.qml`

## Current Behavior
- The layer now also accepts `paperPaletteEnabled`.
  Page/print mode therefore swaps the overlay callout text color away from the previous hardcoded white override and
  keeps the layer aligned with the same paper-safe palette used by the structured callout block editor.

## Responsibility

`ContentsCalloutLayer.qml` renders proprietary `<callout>` source blocks as a Figma-aligned callout row UI.

The layer is renderer-fed:
- input: renderer-owned callout models (`renderedCallouts`)
- output: LVRS callout rows (`background + 1px divider + text`)
- focus hook: `blockFocusHandler(focusSourceOffset)`
- placement hook: `sourceOffsetYResolver(sourceStart)` (host-provided source-offset -> viewport-y resolver)
- host mode flags:
  - `showFrame`
  - `showText`
  - `enableCardFocus`

## Rendering Contract

- Root row style follows Figma node `280:7897`:
  - background `#262728`
  - row padding `4`
  - inner spacing `12`
  - width now stretches across the full resolved editor text column
- Root layer and each callout frame now bind `height: implicitHeight`, so visible callout rows reserve real background
  height instead of collapsing to zero while still carrying child text/divider nodes.
- Left divider:
  - width `1`
  - height spans from top padding to bottom padding, so multi-line callouts keep one continuous bar
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
- Render entries may now be provisional:
  - open-only `<callout>` tags still arrive here
  - `tagVerified=false` means the frame is visible but the source is not yet structurally closed
  - empty callouts still reserve a minimal frame width so blank callout tags do not disappear visually
- The host-provided `sourceOffsetYResolver(...)` now resolves editor-internal document Y, so callout rows stay aligned
  with the live editor content coordinate space.
- The layer normalizes `QVariantList`-like values into JS arrays so C++ renderer properties still produce visible
  callout rows in QML.
- The host can now reuse this layer as background-only chrome (`showText=false`) underneath renderer-owned live editor
  text while preserving the same measured multi-line height.

## Regression Checks

- A source `<callout>One</callout><callout>Two</callout>` must render two callout rows in source order.
- A multi-line source `<callout>Line 1\nLine 2</callout>` must render as one row with expanded background/divider
  height.
- An empty source without `<callout>` tags must produce zero callout rows, but an empty `<callout></callout>` tag must
  still produce one visible callout row.
- A source `<callout>` must still produce one visible provisional callout row.
- Callout rows must be placed at resolved source locations (`sourceOffsetYResolver`) so authored tag position is
  reflected on editor surface.
- Tapping a callout row must route focus back to callout-body start in RAW source.
- Empty or multi-line callouts must still reserve a visible background frame with non-zero height.
- A fill-width host mount must stretch the background and divider across the editor text column instead of collapsing
  to content-hug width.
