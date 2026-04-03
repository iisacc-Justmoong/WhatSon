# `src/app/qml/view/content/editor/ContentsResourceViewer.qml`

## Responsibility

`ContentsResourceViewer.qml` is the dedicated in-editor resource viewer surface used when a selected item is a direct
`.wsresource` package.

## Rendering Contract

- `image` render mode: routes through `ResourceBitmapViewer` and renders inline only when
  `ImageFormatCompatibilityLayer` confirms runtime bitmap decoder compatibility.
- `pdf` render mode: shows an in-app PDF reader through `PdfDocument` + `PdfMultiPageView`.
- other modes: renders no fallback scaffold in this surface.
- Build contract: because this file imports `QtQuick.Pdf`, the app target must link `Qt6::PdfQuick` and, on iOS
  where QML import scanning is disabled for Xcode export generation, the static `Qt6::PdfQuickplugin` QML plugin.

## Inputs

- `resourceEntry`: renderer payload object from `ContentsBodyResourceRenderer`.
  - consumes `displayName`, `type`, `format`, `renderMode`, `source`, `resolvedPath`.
- Internal bridge:
  - `ResourceBitmapViewer` projects `resourceEntry` into bitmap-specific viewer state
    (`viewerSource`, `bitmapRenderable`, `incompatibilityReason`).

## Interaction

- Renders only the resource viewport itself (bitmap image or PDF surface) with no extra metadata strip, button, or fallback card scaffold.

## Tests

- Automated test files are not currently present in this repository.
- Resource-viewer regression checklist for this file:
  - selecting a direct `.wsresource` entry with `renderMode == "pdf"` must not fail QML engine startup because `QtQuick.Pdf` was omitted from the bundle
  - `pdfRenderable` must stay false when the resolved open target is empty so the PDF document does not bind an invalid source during note/resource transitions
  - `image` render mode must continue to render through `ResourceBitmapViewer` without being affected by the PDF dependency wiring
