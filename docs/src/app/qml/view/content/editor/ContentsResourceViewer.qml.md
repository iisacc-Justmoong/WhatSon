# `src/app/qml/view/content/editor/ContentsResourceViewer.qml`

## Responsibility

`ContentsResourceViewer.qml` is the dedicated in-editor resource viewer surface used when a selected item is a direct
`.wsresource` package.

## Rendering Contract

- `image` render mode: routes through `ResourceBitmapViewer` and renders inline only when
  `ImageFormatCompatibilityLayer` confirms runtime bitmap decoder compatibility.
- `pdf` render mode: shows an in-app PDF reader through `PdfDocument` + `PdfMultiPageView`.
- other modes: shows a mode-specific fallback card with an explicit open action.

## Inputs

- `resourceEntry`: renderer payload object from `ContentsBodyResourceRenderer`.
  - consumes `displayName`, `type`, `format`, `renderMode`, `source`, `resolvedPath`.
- Internal bridge:
  - `ResourceBitmapViewer` projects `resourceEntry` into bitmap-specific viewer state
    (`viewerSource`, `bitmapRenderable`, `incompatibilityReason`).

## Interaction

- Preserves a direct open action (`Qt.openUrlExternally(...)`) for unsupported inline render modes.
- Renders a compact header strip so users can confirm resource name/type/format while previewing.
- Shows a compatibility-layer reason string when image render mode is selected but the bitmap
  format is unsupported by the current runtime.
