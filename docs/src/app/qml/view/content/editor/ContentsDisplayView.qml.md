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
- `ContentsStructuredDocumentFlow.qml` now mounts directly from the selected note's RAW snapshot as soon as that
  snapshot is available for the current selection.
- `ContentsDisplayView.qml` no longer keeps a second structured-flow activation latch that waits for
  editor-session-bound confirmation before showing the canonical structured note host.
- Empty selected notes now keep that canonical structured host mounted through one fallback editable prose row, so
  note selection and newly created blank notes no longer disappear into a non-focusable blank center panel.
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
- The non-print editor viewport now reserves a bottom inset that scales up to half of the live editor surface height.
  The last authored line can therefore travel well above the bottom edge instead of collapsing against the shell when
  the user reaches the document tail.
- While the note editor is focused, cursor-row movement and live content-height growth now schedule a viewport
  correction pass that keeps the active typing row inside a mid-viewport typing band.
  As wrapped lines or newly inserted lines push the caret downward, the host advances `Flickable.contentY` with the
  typing row so the typing height stays visually stable instead of dropping to the bottom edge.
- Structured-flow, resource-render, and legacy-editor geometry changes now request gutter refresh through dedicated
  reasons instead of reusing the focused `line-structure` suppression path.
- Structured `cachedLogicalLineEntries` updates now split logical-metric change from geometry-only change.
  Even when line count and start offsets stay the same, resource/callout/agenda spacing or measured block-height
  changes still trigger a gutter refresh as soon as `contentY` / `gutterContentY` move.
- The line-number column now uses a dedicated `gutterBodyGap` token for its right inset instead of reusing the editor
  text column's `editorHorizontalInset`.
  This keeps note-body left padding unchanged while tightening the visual distance between gutter numbers and the first
  text glyph.
- Note-entry gutter/minimap geometry is now also invalidated per selected note instead of trusting only line-count
  parity.
  Rapid note switches therefore clear stale incremental line caches immediately, queue a fresh structured layout-cache
  rebuild for the newly selected note, and only reuse minimap-derived line geometry once it has been regenerated for
  that same note id.
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
