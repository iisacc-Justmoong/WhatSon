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
- Transitive iOS contract: `PdfMultiPageView.qml` imports `QtQuick.Shapes`, so the same static-plugin bundle must
  also include `Qt6::qmlshapesplugin` or QML engine startup fails before the editor surface loads.
- Defensive iOS contract: the bundle also keeps the shared QML runtime and controls/dialog implementation plugins
  explicitly linked (`qmlplugin`, `modelsplugin`, `workerscriptplugin`, `qtquicktemplates2plugin`,
  `qtquickcontrols2implplugin`, `qtquickcontrols2basicstyleimplplugin`, `qtquickcontrols2iosstyleimplplugin`,
  `qtquickdialogs2quickimplplugin`) because `QT_QML_MODULE_NO_IMPORT_SCAN` prevents Qt from discovering those
  transitive static plugins automatically during Xcode project generation.

## Inputs

- `resourceEntry`: renderer payload object from `ContentsBodyResourceRenderer`.
  - consumes `displayName`, `type`, `format`, `renderMode`, `source`, `resolvedPath`.
- Internal bridge:
  - `ResourceBitmapViewer` projects `resourceEntry` into bitmap-specific viewer state
    (`viewerSource`, `bitmapRenderable`, `incompatibilityReason`).
  - The inline image path now also exposes `imagePixelWidth`, `imagePixelHeight`, and `imageAspectRatio`
    from the loaded bitmap so parent layout containers can size the resource frame from the real asset ratio instead
    of a fixed placeholder viewport.

## Interaction

- Renders only the resource viewport itself (bitmap image or PDF surface) with no extra metadata strip, button, or fallback card scaffold.
- For bitmap resources, the viewer remains presentation-neutral: parent frames decide whether the image should crop,
  fit, or drive outer height. The structured note-body path now uses those intrinsic bitmap dimensions to derive
  full-width, auto-height image blocks.

## Tests

- Automated test files are not currently present in this repository.
- Resource-viewer regression checklist for this file:
  - selecting a direct `.wsresource` entry with `renderMode == "pdf"` must not fail QML engine startup because `QtQuick.Pdf` was omitted from the bundle
  - selecting the same PDF entry on iOS must not fail startup because `PdfMultiPageView.qml` could not resolve `QtQuick.Shapes`
  - selecting the same PDF entry on iOS must not uncover a second-stage missing-plugin failure from the shared QML runtime or controls/dialog implementation chain
  - `pdfRenderable` must stay false when the resolved open target is empty so the PDF document does not bind an invalid source during note/resource transitions
  - `image` render mode must continue to render through `ResourceBitmapViewer` without being affected by the PDF dependency wiring
  - a loaded inline image must publish stable intrinsic pixel dimensions so the parent resource frame can keep body
    height in sync with the actual bitmap aspect ratio
