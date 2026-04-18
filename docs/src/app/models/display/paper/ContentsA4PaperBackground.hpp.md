# `src/app/models/display/paper/ContentsA4PaperBackground.hpp`

## Responsibility

Declares the canonical A4 paper background model shared by page and print editor surfaces.

## Public Contract

- Paper identity
  - `paperStandard`
  - `widthMillimeters`
  - `heightMillimeters`
  - `sizeMillimeters`
  - `aspectRatio`
- Paper visual tokens
  - `canvasColor`
  - `paperBorderColor`
  - `paperColor`
  - `paperHighlightColor`
  - `paperShadeColor`
  - `paperSeparatorColor`
  - `paperShadowColor`
  - `paperTextColor`
- Hook slot
  - `requestRefresh()`

## Signals

- `backgroundChanged`

## Notes

- The object exists in `models/display/paper` so page and print surfaces can bind to one authoritative A4 background
  definition instead of hardcoding A4 geometry and color tokens in multiple renderers.
- The width/height pair is stored in real A4 millimeters (`210 x 297`) and the aspect ratio is derived from those
  dimensions instead of being repeated as an isolated magic constant.
