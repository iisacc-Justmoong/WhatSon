# `src/app/qml/view/content/editor/ContentsResourceRenderCard.qml`

## Responsibility
Shared desktop/mobile resource card for `<resource ...>` note entries.

## Inputs
- `resourceEntry`
  Normalized renderer payload from `ContentsBodyResourceRenderer`.
- Internal bitmap fallback:
  - also instantiates `ResourceBitmapViewer` locally so the shared card can recognize resolved bitmap assets even when
    the upstream payload forgot to keep `renderMode == "image"`
- `borderColor`, `cardColor`
  Host-supplied tone overrides so desktop/mobile can keep their existing overlay palette.
  The shared defaults now use neutral LV panel colors (`panelBackground08` border plus `panelBackground03` fill)
  instead of the older accent-leaning hard-coded overlay tint, so inline resource-adjacent chrome stays aligned with
  the Figma image-frame palette.
- `inlinePresentation`
  Switches the card into the compact in-editor block mode used by `ContentsResourceLayer.qml`, where metadata stays
  single-line and the card height matches the reserved editor placeholder slot.
  Inline bitmap images now also have a second presentation mode:
  - compact non-image resources still use the metadata card
  - inline image resources suppress the wrapper-card chrome and hand the full slot to
    `ContentsImageResourceFrame.qml` + `ContentsResourceViewer.qml`, so the note body shows the bitmap itself instead
    of a thumbnail-summary tile
  The card now also binds `height` to that computed inline/regular implicit height explicitly, so `Repeater`-mounted
  resource delegates own real paint area instead of relying on layout-only implicit sizing.

## Render Modes
- `image`
  Shows either an inline bitmap preview card or, in inline note-body mode, a full-slot bitmap viewer without metadata
  chrome.
- `document` plus bitmap fallback
  If the renderer payload downgraded an actual bitmap resource to `document` but still resolved a compatible bitmap
  file path, the shared card now still promotes that entry into the same Figma `292:50` image-frame treatment.
  The generic `Document Resource` summary card itself no longer exists; unsupported `document` entries now collapse
  instead of inventing a metadata tile that is not part of the design.
- `text`
  Shows a trimmed text snippet preview.
- `video`, `audio`, `pdf`
  Show a typed placeholder tile plus metadata and an external-open action.

## Reuse Rule
- Desktop `ContentsDisplayView.qml` and mobile `MobileContentsDisplayView.qml` must both compose this shared card
  instead of reintroducing duplicated per-host resource-card markup.
- The same card now serves both:
  - inline note-body rendering through `ContentsResourceLayer.qml`
  - non-inline resource presentation surfaces that keep the previous freer vertical sizing
- The inline image path now follows the Figma `292:50` resource frame more closely:
  - outer frame stays transparent and keeps only the border
  - default frame chrome is neutral panel/caption color, not Accent
  - the outer image frame stretches to the full available body width only
  - it must not treat gutter/minimap space or raw viewport overhang as part of that width contract
  - the inner bitmap viewport stays centered inside that full-width frame
  - in note-body inline mode, the bitmap may shrink to fit while preserving aspect ratio, but it must not upscale
  - the image area still respects the real bitmap aspect ratio, but the inline frame now caps the body-block media
    height to the Figma `292:50` reference budget so a tall image cannot monopolize the whole editor surface
