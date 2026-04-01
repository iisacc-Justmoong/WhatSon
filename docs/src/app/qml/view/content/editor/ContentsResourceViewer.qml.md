# `src/app/qml/view/content/editor/ContentsResourceViewer.qml`

## Responsibility

`ContentsResourceViewer.qml` is the dedicated in-editor resource viewer surface used when a selected item is a direct
`.wsresource` package.

## Rendering Contract

- `image` render mode: routes through `ResourceBitmapViewer` and renders inline only when
  `ImageFormatCompatibilityLayer` confirms runtime bitmap decoder compatibility.
- `pdf` render mode: shows an in-app PDF reader through `PdfDocument` + `PdfMultiPageView`.
- other modes: renders no fallback scaffold in this surface.

## Inputs

- `resourceEntry`: renderer payload object from `ContentsBodyResourceRenderer`.
  - consumes `displayName`, `type`, `format`, `renderMode`, `source`, `resolvedPath`.
- Internal bridge:
  - `ResourceBitmapViewer` projects `resourceEntry` into bitmap-specific viewer state
    (`viewerSource`, `bitmapRenderable`, `incompatibilityReason`).

## Interaction

- Renders only the resource viewport itself (bitmap image or PDF surface) with no extra metadata strip, button, or fallback card scaffold.
