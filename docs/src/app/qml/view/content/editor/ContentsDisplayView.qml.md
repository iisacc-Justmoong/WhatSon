# `src/app/qml/view/content/editor/ContentsDisplayView.qml`

## Responsibility

`ContentsDisplayView.qml` is the unified desktop/mobile note editor host.

It composes:

- selection/session sync
- whole-document presentation refresh policy
- structured document-flow mounting
- legacy fallback editor hosting
- gutter/minimap/page layout
- resource import wiring

It no longer acts as the direct center-surface viewer for resource-backed hierarchy browsing.
`ContentViewLayout.qml` now mounts `ContentsResourceEditorView.qml` beside this host whenever the active list model is
not note-backed.

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
- `ContentsDisplayView.qml` now copies projection logical-line metrics into explicit host state instead of relying on
  breakable QML property bindings.
- Projection `logicalLineCount` and `logicalLineStartOffsets` changes now schedule gutter refresh directly, so line
  numbers no longer wait for an unrelated cursor, scroll, or layout event before updating.
- `commitDocumentPresentationRefresh()` refreshes only the HTML overlay/minimap projection; it no longer triggers a
  RichText surface reinjection step.
- Resource-bearing fallback notes still substitute `whatson-resource-block` placeholders into HTML, but that
  substitution now stays entirely inside the display pipeline.
- The non-print editor viewport now reserves a multi-line bottom inset, so the last authored line no longer sits flush
  against the shell bottom edge when the user scrolls to the document tail.
- Structured-flow, resource-render, and legacy-editor geometry changes now request gutter refresh through dedicated
  reasons instead of reusing the focused `line-structure` suppression path.
- Structured `cachedLogicalLineEntries` updates now split logical-metric change from geometry-only change.
  Even when line count and start offsets stay the same, resource/callout/agenda spacing or measured block-height
  changes still trigger a gutter refresh as soon as `contentY` / `gutterContentY` move.
- Page/print mode now also injects `paperPaletteEnabled` into both `ContentsEditorPresentationProjection` and
  `ContentsStructuredDocumentFlow.qml`, so the white paper surface cannot inherit dark-theme body white from either the
  whole-document HTML renderer or the structured block delegates.

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
