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
- `ContentsDisplayNoteBodyMountCoordinator` now owns the note-body mount contract between the selection bridge,
  the editor session, and the mounted document surface.
  The host stays in a pending mount state until either the selected note snapshot resolves for that same note id or
  the editor session binds directly to the selected note.
- If no note is selected, the host suppresses the body chrome and shows the centered `No document opened` placeholder
  plus a short instruction to pick a note from the list.
- If one snapshot refresh retry still leaves the selected note body unresolved or mismatched, the host now shows a
  reason-specific centered placeholder instead of collapsing every failure into `No document opened`.
  Current failure labels distinguish at least:
  - body snapshot mismatch against the current selection
  - unresolved note body after snapshot refresh
  - structured document surface not becoming ready
  - inline fallback editor surface not becoming ready
- Structured shortcut insertions now try the parser-owned document flow first, but if that host cannot resolve a live
  caret anchor they fall back to the legacy cursor bridge instead of appending at the document tail.

## Debug Trace

- The host now emits explicit editor-creation trace turns for note selection, body-note resolution, editor-session
  binding, document mount pending/mounted/failure transitions, structured-host requests, and legacy-inline-host
  requests.
- It now also emits note-selection flow plan traces end-to-end:
  - `selectionFlow.scheduleSelectionModelSync` when the host translates a note selection/body update into paired
    selection-sync and mount passes
  - `selectionFlow.pollPlan` / `selectionFlow.pollReconcileRequested` / `selectionFlow.pollSnapshotRefresh` for
    periodic same-note snapshot comparison
  - `selectionFlow.reconcilePlan` / `selectionFlow.reconcileRequested` for one-shot editor-entry reconciliation
  - `selectionFlow.selectionSyncPlan` / `selectionFlow.selectionSyncResult` for editor-session sync, cache reset,
    focus, and fallback-refresh handling emitted from `ContentsDisplaySelectionSyncCoordinator`
  - `selectionFlow.mountPlan` / `selectionFlow.mountResult` for the actual mount/snapshot-refresh/editor-bind outcome
- `scheduleSelectionModelSync(...)` now dispatches the same normalized option payload into both
  `ContentsDisplaySelectionSyncCoordinator` and `ContentsDisplayNoteBodyMountCoordinator`.
  The host also keeps those flush handlers on separate `Connections` blocks so a mount-plan signal cannot be lost by
  being attached to the wrong coordinator target.
- The inline loader now logs both `contentEditorLoaderStatusChanged` and `contentEditorLoaderLoaded`, including the
  current loader status plus a summarized view of the instantiated editor item.
- The structured host path now logs `structuredDocumentFlowVisibleChanged` so it is visible exactly when the parsed
  document surface becomes the live editor surface.
- Host-owned creation logs now assign stable object names to the main editor-collaboration objects:
  - `contentsDisplaySelectionBridge`
  - `contentsDisplayNoteBodyMountCoordinator`
  - `contentsDisplaySessionCoordinator`
  - `contentsDisplayEditorSession`
  - `contentsDisplayStructuredDocumentFlow`
  - `contentsDisplayInlineEditorLoader`
  - `contentsDisplayInlineFormatEditor`
- Those names are mirrored in the QML trace payload so startup/runtime logs can reconstruct which object instance was
  created, destroyed, or selected as the active document surface during editor bootstrap.

## Presentation Refresh

- `ContentsEditorPresentationProjection` now exposes editor HTML, preview HTML, and logical text/line metadata.
- `ContentsDisplayView.qml` now copies projection logical-line metrics into explicit host state instead of relying on
  breakable QML property bindings.
- Projection `logicalLineCount` and `logicalLineStartOffsets` changes now schedule gutter refresh directly, so line
  numbers no longer wait for an unrelated cursor, scroll, or layout event before updating.
- `resolvedDocumentPresentationSourceText()` now accepts same-note `selectedNoteBodyText` even while the bridge is
  still finishing a background snapshot refresh.
  This prevents the structured document host from mounting with a transient empty source when the selected note is
  already known but the editor session binding has not completed yet.
- The document-source resolver itself now lives under `src/app/models/editor/display`.
  QML still binds live selection/session state into it, but source arbitration is now treated as editor-domain C++
  logic rather than as a content-panel helper.
- The `ContentsDisplayDocumentSourceResolver` block keeps each upstream source property bound exactly once.
  Duplicate assignments in that block are a hard QML compile error, so the host now relies on one normalized binding
  per resolver input and the regression suite checks that those keys remain unique.
- Logical-line offset lookup and minimap viewport math now route through
  `ContentsDisplayViewportCoordinator` in `src/app/models/editor/display`.
  `ContentsDisplayView.qml` still owns cache invalidation and composition, but it no longer carries those binary
  searches and proportional-track calculations as local JavaScript helpers.
- `commitDocumentPresentationRefresh()` refreshes only the HTML overlay/minimap projection; it no longer triggers a
  RichText surface reinjection step.
- Resource-bearing fallback notes still substitute `whatson-resource-block` placeholders into HTML, but that
  substitution now stays entirely inside the display pipeline.
- The non-print editor viewport now reserves the existing bottom accessibility inset at up to roughly half of the
  live editor surface height, so the last authored line can be pulled much higher above the shell bottom edge when the
  user scrolls to the document tail.
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
