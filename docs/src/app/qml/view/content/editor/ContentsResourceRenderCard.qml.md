# `src/app/qml/view/content/editor/ContentsResourceRenderCard.qml`

## Responsibility
Shared desktop/mobile body-overlay resource card for `<resource ...>` note entries.

## Inputs
- `resourceEntry`
  Normalized renderer payload from `ContentsBodyResourceRenderer`.
- `borderColor`, `cardColor`
  Host-supplied tone overrides so desktop/mobile can keep their existing overlay palette.

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
