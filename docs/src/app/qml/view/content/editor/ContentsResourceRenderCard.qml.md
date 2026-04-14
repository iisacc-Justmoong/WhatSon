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
  file path, the shared card now still promotes that entry into the same Figma `292:50` image-frame treatment instead
  of falling back to the empty metadata summary tile.
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
- The inline image path now follows the Figma `292:50` resource frame more closely:
  - outer frame stays transparent and keeps only the border
  - the outer image frame stretches to the full available body width only
  - it must not treat gutter/minimap space or raw viewport overhang as part of that width contract
  - the inner bitmap viewport stays centered and does not upscale beyond the natural bitmap width hint
  - in note-body inline mode, the inner bitmap now renders with `PreserveAspectCrop` so the frame reads like the Figma
    mixed prose/image example instead of letterboxing inside the centered media slot
  - the image area height tracks the real bitmap aspect ratio instead of a fixed 480px-wide sample card
