# `src/app/qml/view/content/editor/ContentsDisplayView.qml`

## Responsibility

`ContentsDisplayView.qml` is the unified desktop/mobile editor host.

It composes:

- selection/session sync
- whole-document presentation refresh policy
- structured document-flow mounting
- legacy fallback editor hosting
- gutter/minimap/page layout
- resource import wiring

## Editing Model

- RAW `.wsnbody` remains the only write authority.
- `ContentsStructuredDocumentFlow.qml` is the canonical note host once a note session is bound.
- The fallback whole-note editor path now uses:
  - plain logical text as the live input buffer
  - tokenized HTML as a separate read-side overlay (`renderedEditorHtml`)
- The host no longer pushes a RichText editing surface back into `ContentsInlineFormatEditor.qml`.
- Structured shortcut insertions now try the parser-owned document flow first, but if that host cannot resolve a live
  caret anchor they fall back to the legacy cursor bridge instead of appending at the document tail.

## Presentation Refresh

- `ContentsEditorPresentationProjection` now exposes editor HTML, preview HTML, and logical text/line metadata.
- `commitDocumentPresentationRefresh()` refreshes only the HTML overlay/minimap projection; it no longer triggers a
  RichText surface reinjection step.
- Resource-bearing fallback notes still substitute `whatson-resource-block` placeholders into HTML, but that
  substitution now stays entirely inside the display pipeline.
- The non-print editor viewport now reserves a multi-line bottom inset, so the last authored line no longer sits flush
  against the shell bottom edge when the user scrolls to the document tail.

## Resource Import

- Resource insertion still mutates RAW source first.
- Canonical RAW `<resource ... />` text generation now lives behind the `ContentsResourceTagTextGenerator` C++ bridge
  instead of a host-local JavaScript string builder.
- The same import path now applies resource insertion payloads through `applyDocumentSourceMutation(...)`, so
  structured-flow focus restore and persistence scheduling happen on the same host RAW mutation path as other editor
  rewrites.
- After import, the host restores the plain-text editor surface from projection logical text and recomputes the HTML
  overlay.
- Inline resource preview HTML is therefore presentation-only and cannot become editor write authority.
