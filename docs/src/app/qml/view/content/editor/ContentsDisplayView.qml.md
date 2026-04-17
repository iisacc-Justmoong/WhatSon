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

## Presentation Refresh

- `ContentsEditorPresentationProjection` now exposes editor HTML, preview HTML, and logical text/line metadata.
- `commitDocumentPresentationRefresh()` refreshes only the HTML overlay/minimap projection; it no longer triggers a
  RichText surface reinjection step.
- Resource-bearing fallback notes still substitute `whatson-resource-block` placeholders into HTML, but that
  substitution now stays entirely inside the display pipeline.

## Resource Import

- Resource insertion still mutates RAW source first.
- Canonical RAW `<resource ... />` text generation now lives behind the `ContentsResourceTagTextGenerator` C++ bridge
  instead of a host-local JavaScript string builder.
- After import, the host restores the plain-text editor surface from projection logical text and recomputes the HTML
  overlay.
- Inline resource preview HTML is therefore presentation-only and cannot become editor write authority.
