# `src/app/qml/view/content/editor/ContentsResourceViewer.qml`

## Responsibility

`ContentsResourceViewer.qml` is the dedicated in-editor resource viewer surface used when a selected item is a direct
`.wsresource` package.

## Rendering Contract

- `image` render mode: shows an in-app bitmap preview through `Image`.
- `pdf` render mode: shows an in-app PDF reader through `PdfDocument` + `PdfMultiPageView`.
- other modes: shows a mode-specific fallback card with an explicit open action.

## Inputs

- `resourceEntry`: renderer payload object from `ContentsBodyResourceRenderer`.
  - consumes `displayName`, `type`, `format`, `renderMode`, `source`, `resolvedPath`.

## Interaction

- Preserves a direct open action (`Qt.openUrlExternally(...)`) for unsupported inline render modes.
- Renders a compact header strip so users can confirm resource name/type/format while previewing.
