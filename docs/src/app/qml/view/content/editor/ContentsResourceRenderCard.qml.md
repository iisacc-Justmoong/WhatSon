# `src/app/qml/view/content/editor/ContentsResourceRenderCard.qml`

## Responsibility
Shared desktop/mobile resource card for `<resource ...>` note entries.

## Inputs
- `resourceEntry`
  Normalized renderer payload from `ContentsBodyResourceRenderer`.
- `borderColor`, `cardColor`
  Host-supplied tone overrides so desktop/mobile can keep their existing overlay palette.
- `inlinePresentation`
  Switches the card into the compact in-editor block mode used by `ContentsResourceLayer.qml`, where metadata stays
  single-line and the card height matches the reserved editor placeholder slot.

## Render Modes
- `image`
  Shows an inline bitmap preview.
- `text`
  Shows a trimmed text snippet preview.
- `video`, `audio`, `pdf`, `document`
  Show a typed placeholder tile plus metadata and an external-open action.

## Reuse Rule
- Desktop `ContentsDisplayView.qml` and mobile `MobileContentsDisplayView.qml` must both compose this shared card
  instead of reintroducing duplicated per-host resource-card markup.
- The same card now serves both:
  - inline note-body rendering through `ContentsResourceLayer.qml`
  - non-inline resource presentation surfaces that keep the previous freer vertical sizing
