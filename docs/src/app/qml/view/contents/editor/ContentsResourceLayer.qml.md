# `src/app/qml/view/contents/editor/ContentsResourceLayer.qml`

## Responsibility
Projects note-body `<resource ... />` entries into inline editor coordinates and renders a framed resource card at each
tag location when that resource is still using the overlay/fallback presentation path.

## Inputs
- `renderedResources`
  `ContentsBodyResourceRenderer` payload that now includes `sourceStart`, `sourceEnd`, and `focusSourceOffset`.
- `sourceOffsetYResolver`
  Host callback that converts a source offset into the current editor document Y coordinate.
- `blockFocusHandler`
  Optional host callback used when the user taps the inline resource frame.
- `borderColor`, `cardColor`
  Host palette overrides so desktop/mobile keep their current surface-specific colors.

## Behavior
- Reuses `ContentsResourceRenderCard.qml` in `inlinePresentation` mode instead of duplicating resource-card markup.
- Normalizes rendered-resource lists from native JS arrays, Qt sequence wrappers with `length`/`count`, and numeric-key
  object façades.
  The overlay layer therefore still paints inline resource frames when the renderer payload reaches QML as a wrapped
  `QVariantList` instead of a native array.
- Computes each card Y position from `sourceStart`, matching the same overlay contract already used by agenda/callout
  layers.
- Exposes `resourceCount` and `implicitHeight` so the host can gate visibility without a second list transform.
- Desktop/mobile RichText hosts may now pass only the subset of rendered resources that are not yet upgraded into
  inline editor HTML blocks.
