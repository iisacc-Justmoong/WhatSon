# `src/app/qml/view/content/editor/ContentsDisplayView.qml`

## Responsibility
Desktop content editor host.

## Editor Input Mode
- The desktop note editor now runs its mounted `ContentsInlineFormatEditor.qml` in plain-text mode only.
  `editorSession.editorText` / `textMetricsBridge.logicalText` remain the live editable surface, while HTML conversion
  stays in the renderer layer and is no longer rebound into the editable `TextEdit` itself.
- This removes the last active desktop `TextEdit.RichText` dependency from note typing, which prevents Qt RichText
  document scaffolds from becoming part of the write path during ordinary editing.
- `commitDocumentPresentationRefresh()` still updates `ContentsTextFormatRenderer.sourceText` on the same turn, so the
  render-layer HTML projection keeps following RAW source immediately even though the editable `TextEdit` itself now
  stays plain text.

## Structured Document Flow
- `agenda`, `callout`, `resource`, and `break` remain `.wsnbody` body tags, and desktop now lets any parsed explicit
  document block, including semantic text-tag blocks and `resource`, activate `ContentsStructuredDocumentFlow.qml`.
  A note whose RAW body contains `<resource ... />` therefore renders that block through the parser-owned document flow
  instead of relying on the legacy inline-editor overlay path.
- Canonical live `<resource ... />` markup is still tracked through the live editor buffer, presentation snapshot, and
  selection-bridge snapshot so `ContentsBodyResourceRenderer` can resolve inline resources against the freshest RAW
  source.
- `ContentsStructuredDocumentFlow.qml` therefore becomes the host for agenda/callout/break/resource notes and for notes
  whose RAW still carries explicit semantic text tags whenever the parser reports explicit document blocks.
- That block stream now also includes parser-owned semantic text blocks for `paragraph` / `title` / `subTitle` /
  `event*` body markup.
  A note that mixes prose paragraphs with `<resource ... />` tags therefore keeps both the prose and the inline image
  frame in the same document-owned flow, instead of swapping to a resource-only surface that leaves the text empty.
- Structured-flow visibility is now additionally gated by `editorSession.editorBoundNoteId == selectedNoteId`.
  A previously rendered structured note therefore cannot remain mounted after the user selects a different note-list
  item; the host hides stale block content until the newly selected note session is actually bound.
- Structured-flow activation is now also gated by `structuredBlockRenderer.hasRenderedBlocks` only.
  A live `<resource ... />` token by itself no longer flips the host into block mode before the parser has produced the
  full mixed-content document block list, so prose paragraphs are not displaced by a premature image-only surface.
- Source persistence for block edits now runs through `applyDocumentSourceMutation(...)`, which updates the RAW body,
  marks local authority, optionally restores focus inside the reparsed block, and only forces a full legacy
  presentation rebuild when the note actually falls back out of structured-flow mode.
- While structured-flow mode is active, legacy agenda/callout overlay layers now receive empty models so hidden fallback
  delegates do not instantiate in parallel with the document-native block flow.
- The same structured-flow surface now also receives `bodyResourceRenderer.renderedResources`, so a `<resource ... />`
  block can resolve the actual asset path from inside the referenced `.wsresource` bundle and paint the existing
  `ContentsImageResourceFrame` card inline in document order.
- In screen editor mode, that structured-flow column now uses the same effective body width contract as ordinary note
  text: viewport width minus the editor's left/right body inset.
  Inline image resources therefore fill the note body column itself, not the whole editor viewport, and do not intrude
  into gutter/minimap-adjacent space.
- Desktop host no longer mounts a dedicated `ContentsResourceViewer` surface for note editing.
  Inline `<resource ... />` tags must stay inside `ContentsStructuredDocumentFlow.qml` as ordinary document blocks,
  and note hosts no longer switch to a resource-only surface or resource overlay layer.
- While structured-flow mode is active, a desktop left-click anywhere in the structured document viewport now routes
  through `requestStructuredDocumentEndEdit()`. That click re-enters editing at the document tail instead of leaving
  the user on a non-editable resource/break block focus.
- Note-open reconcile is now scheduled through a deferred one-shot helper instead of running synchronously in the
  `selectedNoteIdChanged` turn.
- The host now tracks one pending note-entry reconcile id and waits for
  `selectionBridge.viewSessionSnapshotReconciled(...)` before marking that note's entry snapshot as compared.
- Timer-driven snapshot polling now treats reconcile as an asynchronous request; the eventual `selectedNoteBodyText`
  echo and reconcile-complete signal drive follow-up UI refresh work.
- Timer-driven polling now also reuses that pending note-entry id, so one selected note does not enqueue overlapping
  reconcile requests while a previous worker fetch is still in flight.
- Model-to-editor body sync and reconcile-complete handling now funnel heavy UI refresh work through
  `editorSession.editorTextSynchronized` instead of scheduling the same minimap/presentation/gutter refresh sequence
  from multiple handlers.
- Desktop note entry now also treats gutter recomputation as a first-class note-open step.
  `selectedNoteIdChanged` immediately clears the previous note's visible gutter-line model and line-geometry cache,
  then `editorSession.editorTextSynchronized` forces a fresh gutter rebuild for the newly bound note body.
  Gutter line numbers and Y positions therefore no longer wait for incidental scroll/resize/editor metrics changes
  before leaving the previous note's geometry behind.
- Selection-driven editor sync now also funnels through one queued `scheduleSelectionModelSync(...)` helper:
  `selectedNoteIdChanged`, `selectedNoteBodyTextChanged`, initial mount, and visibility re-entry all merge into one
  `requestSyncEditorTextFromSelection(...)` turn with shared snapshot-reset, reconcile, fallback-refresh, and
  focus-restoration flags.
- The queued selection-sync helper keeps the `requestSyncEditorTextFromSelection(...)` result in one local gate before
  scheduling fallback presentation refresh, preserving the deferred sync behavior without relying on parser-hostile QML
  identifier names.
- Desktop note-open now also waits for `selectionBridge.selectedNoteBodyLoading == false` before syncing the selected
  body into `ContentsEditorSession`.
  While that lazy load is pending, the host keeps the previous editor buffer intact, can request an immediate fetch
  attempt for the previously bound note, disables the editor viewport, and shows a loading overlay instead of pushing
  an empty interim body into the editor.
- Desktop note-open now also re-queues `scheduleSelectionModelSync(...)` when
  `selectedNoteBodyLoading` flips back to `false`, even if the loaded body text is still the empty string.
  Empty-body notes therefore clear the stale previous note session instead of leaving the old body mounted just because
  `selectedNoteBodyTextChanged` had no new payload to emit.
- The host now also requires `selectionBridge.selectedNoteBodyNoteId == selectedNoteId` before syncing the body into
  the session or restoring editor focus.
  This prevents one note's stale body text from being rebound under another note id after a failed or superseded lazy
  body fetch.
- During a note-selection transition, the desktop host no longer asks
  `ContentsEditorTypingController.handleEditorTextEdited()` to diff the editor surface after `selectedNoteId` has
  already changed.
  Once the bound session note and selected note diverge, the host waits for same-note rebinding before any further
  diff/persist work, preventing one note's stale editor surface from being serialized into another note.
- That deferred blur-side flush now also captures the note id that owned the editor when focus was lost and refuses to
  run after the session has rebound to a different selected note.
  Clicking from note `A` to note `B` therefore cannot reinterpret `A`'s stale editor surface as a late edit for `B`
  and overwrite the newly selected note body during the transition.
- The same blur-side flush now additionally requires an active local authoring session or a pending body-save for that
  same bound note before it runs at all.
  A plain note-list selection click on an untouched note therefore cannot promote the freshly selected note into a
  persistence turn just because focus left the editor.
- Timer-driven snapshot polling and one-shot entry reconcile now also require both of these same-note ownership checks
  before sending `editorSession.editorText` to the selection bridge:
  - `editorSession.editorBoundNoteId == selectedNoteId`
  - `selectionBridge.selectedNoteBodyNoteId == selectedNoteId`
  A newly selected note therefore cannot be reconciled against the previously bound note's stale session text.
- Desktop inline-editor `onTextEdited()` now only notifies the typing controller to read the live `TextEdit` state.
  The host no longer treats the rendered RichText surface payload itself as a recovery source for RAW note text, so
  agenda/callout projection cannot push the note-open session snapshot back over newer visible edits.
- Desktop now routes Backspace/Delete through `ContentsEditorTypingController.handleTagAwareDeleteKeyPress(...)`
  before the generic shortcut controller and before `TextEdit` default deletion.
  Raw tag tokens therefore disappear atomically when the caret touches them, instead of being nibbled one source
  character at a time.
- Desktop host-side RAW mutations such as imported-resource tag insertion or structured source rewrites now also issue
  immediate persistence requests directly; the host no longer keeps a local deferred-persistence override for those
  editor mutations.
- Desktop editor drop handling now accepts native file-manager drags without a `DropArea.keys` MIME gate, parses
  `drop.urls` first and then falls back through string-based payloads such as `drop.text`, `text/uri-list`,
  `text/plain`, and platform file-url MIME values, imports those files through `ResourcesImportViewModel`, injects
  canonical `<resource ...>` calls into the active note source, and feeds the current presentation snapshot into
  `ContentsBodyResourceRenderer` so the dropped resource card appears in the body overlay before the worker-thread note
  flush finishes.
- Before that import actually mutates storage, desktop now asks `ResourcesImportViewModel.inspectImportConflictForUrls(...)`
  or `inspectClipboardImageImportConflict()` whether the incoming asset name already exists.
  If it does, the editor opens an `LV.Alert` with `Overwrite`, `Keep Both`, and `Cancel Import` actions instead of
  silently auto-numbering the new package.
- Desktop now also rescans the live `editorText` buffer for canonical `<resource ... />` markup on each edit turn.
  That keeps resource resolution tied to the latest RAW source while still allowing parser-owned resource blocks to
  activate the structured document flow on the same selected note.
- The host now also exposes one helper that counts canonical `<resource ... />` tokens in a RAW snapshot and another
  helper that flags resource-tag loss while the note is still on the legacy inline editor.
  That loss check compares the candidate rewrite against the strongest currently known RAW baseline
  (`editorText`, `documentPresentationSourceText`, and the selected-note body snapshot only when that snapshot still
  belongs to the selected note and the host has not taken local editor authority), so a stale surface rewrite is
  rejected even if one buffer had already drifted once without treating an older model snapshot as newer than a just-
  accepted local edit.
- `applyDocumentSourceMutation(...)` now also runs that same loss check before it mutates `editorText` or persists.
  Any caller that bypasses the typing/selection controllers therefore still cannot silently delete `<resource ... />`
  from RAW while the host remains on the legacy inline-editor surface.
- Figma node `294:7933` is now the mixed-content reference for this path:
  one text-editor column contains prose above the image frame and prose below it.
  Inline image blocks therefore behave like authored body elements, not like a full-surface resource takeover.
- That mixed-content reference also means the structured-flow document column should read like an ordinary markdown/HTML
  note body:
  - text/resource blocks keep visible vertical breathing room between siblings
  - body prose keeps the denser 12px medium-weight paragraph feel instead of oversized editor-field spacing
- The post-import insertion path now normalizes `importUrlsForEditor(...)` results from either a real JS array or a
  Qt-provided list-like `QVariantList`, so successful hub imports cannot silently skip body-tag insertion just because
  the invokable return value is not tagged as `Array.isArray(...)` in QML.
- Desktop resource-drop insertion now always emits canonical self-closing `<resource ... />` source with quoted,
  XML-escaped attributes, including quoted relative `path=".../.wsresource"` values.
- Desktop no longer splices that imported resource tag block by the raw `TextEdit.cursorPosition` integer.
  On the legacy inline editor path it routes insertion through
  `ContentsEditorTypingController.insertRawSourceTextAtCursor(...)`, so logical/plain caret positions are mapped back
  into `.wsnbody` source offsets before persistence.
- Desktop now also routes plain `Enter` / `Return` through
  `ContentsEditorTypingController.handlePlainEnterKeyPress(...)` before falling back to delete/formatting shortcuts.
  Line splits on the legacy inline editor therefore mutate RAW source directly instead of trusting a transient
  RichText paragraph rewrite.
- When the note is already in `ContentsStructuredDocumentFlow.qml` because of agenda/callout/break blocks, desktop now
  inserts dropped resource tags through the structured-flow host's active-block insertion path instead of falling back
  to the legacy inline-editor cursor bridge.
- Desktop now also normalizes that inserted resource block onto standalone source lines when the drop happened in the
  middle of an existing paragraph, so the inline resource frame owns its own body slot instead of being embedded into
  adjacent prose.
- Desktop now requests the resources runtime reload only after the same drop turn finishes its RAW note-link attempt.
  Import therefore no longer reloads the resources hierarchy midway through the editor-linking step, but successful
  `.wsresource` registration still refreshes the runtime even when the note-link step fails.
- Desktop RichText editor presentation still upgrades `whatson-resource-block` placeholders into paragraph-oriented
  document blocks before that HTML is bound into `ContentsInlineFormatEditor`.
- Once the selected note has entered structured-flow mode, desktop now stops relying on that RichText image upgrade
  path for inline resource display and instead renders `<resource ... />` through `ContentsResourceBlock.qml` inside
  `ContentsStructuredDocumentFlow.qml`.
- `ContentsBodyResourceRenderer`, `ContentsStructuredBlockRenderer`, and `ContentsStructuredDocumentFlow` now share one
  `structuredFlowSourceText` contract.
  Resource-block activation therefore stays aligned even when the live editor surface, the deferred presentation
  snapshot, and the selection-bridge body source are temporarily catching up with each other.
- Desktop now also gates `ContentsBodyResourceRenderer.bodySourceText` by
  `editorSession.editorBoundNoteId == selectedNoteId` instead of by the selection-bridge body-note echo alone.
  A same-note drag/drop or clipboard image insert therefore resolves the just-inserted `<resource ... />` tag from the
  live bound editor RAW immediately, without waiting for the persisted-body snapshot to catch up first.
- Desktop now also binds `ContentsBodyResourceRenderer.noteDirectoryPath` from
  `selectionBridge.selectedNoteDirectoryPath`.
  Inline resource rendering therefore uses the same resolved note package path as the mounted editor session instead of
  depending solely on the active hierarchy view-model to answer `noteDirectoryPathForNoteId(...)` again.
- The desktop editable note surface now keeps RichText inline-image upgrading disabled by default.
  Resolved bitmap resources therefore render only through parser-owned document blocks inside
  `ContentsStructuredDocumentFlow.qml`, which keeps authored prose and image frames in one block stream and avoids
  editor-surface overlays entirely.
- Desktop gutter and minimap rails now stay mounted for structured-flow notes as well.
  The host no longer treats `<resource ... />` notes as a special full-surface rendering mode that must disable editor
  chrome; instead it asks `ContentsStructuredDocumentFlow.qml` for block-derived logical line entries and feeds those
  lines back into the existing gutter/minimap pipeline.
- The desktop gutter now packs only the currently visible structured lines into one editor-row-per-line gutter slots.
  Tall image/divider blocks therefore keep their real document height and minimap silhouette, but the left gutter no
  longer expands one bitmap card into several apparent line-number rows.
- That gutter path now also maps the live scroll position from actual document Y into the compressed gutter Y space.
  Scrolling through the middle of a tall image therefore moves the gutter rows continuously instead of freezing them at
  one packed slot until the block leaves the viewport.
- The structured minimap path now also preserves block-style rows for inline images.
  A resource card's minimap silhouette therefore reads like one tall filled body block instead of many narrow
  text-width bars.
- Desktop note transitions now also keep structured-flow notes off the legacy whole-editor typing diff path.
  Switching away from a note whose body contains parser-owned `resource` blocks therefore no longer lets the fallback
  inline-editor serializer rewrite or damage `<resource ... />` source during blur/selection-change cleanup.
  The same cleanup path now also aborts when the legacy inline editor is not actually mounted, so selection changes
  cannot diff against the null-safe proxy object and accidentally rewrite the previously bound note source.
- `ContentsBodyResourceRenderer` now also receives `libraryHierarchyViewModel` as a fallback note-directory resolver,
  so note-body resource rendering survives hierarchy-switch lag or active domains that do not implement
  `noteDirectoryPathForNoteId(QString)`.
- RichText dirtiness checks now compare against the already-upgraded inline-resource HTML, so the desktop host no
  longer treats every resource-bearing note as permanently dirty just because `editorSurfaceHtml` still contains the
  placeholder marker payload.
- Desktop host no longer uses `ContentsResourceLayer.qml` for note-body rendering.
  Resource visibility now depends on parsed document blocks, not on a second overlay pass above the editor surface.
- When the selection bridge can already expose a buffered dirty body for the newly selected note, the desktop host now
  consumes that note-owned payload through the ordinary selection-sync path instead of waiting for a stale filesystem
  read to arrive first.
- While structured-flow mode is active, the legacy `ContentsInlineFormatEditor` now unloads entirely through a `Loader`
  instead of remaining alive behind `visible: false`.
- The host keeps a lightweight proxy object under the existing `contentEditor` reference so shared geometry/focus helpers
  can keep null-safe access patterns even while the legacy editor instance is absent.
- Host-side selection and typing controllers now bind to `contentsView.contentEditor` explicitly instead of relying on a
  self-referential `contentEditor: contentEditor` assignment that produced runtime binding loops under bound-component
  semantics.
- Desktop note-open now keeps the legacy inline editor path alive only until the first settled structured render confirms
  that the currently selected note actually owns parsed explicit document blocks such as agenda/callout/break/resource
  or semantic text-tag blocks.
- After that first same-note activation, later async reparses keep the structured-flow surface mounted instead of
  bouncing back through the legacy editor during in-block editing.
- Timer-driven note snapshot polling now also pauses while the selected note body is still loading, so note-open does
  not compete with an overlapping same-note refresh probe.
- Desktop no longer auto-mounts `ContentsStructuredTagValidator` as a parser-driven write path.
  Renderer-side correction suggestions may still exist internally, but note-open and typing now stay on the single
  editor-session persistence path instead of opening an extra validator-triggered file write + note-list refresh turn.
- The print-document `Repeater` delegate now declares `required property int index`, and the inline editor host uses a
  literal `shapeStyle: 0`, removing runtime `ReferenceError` noise seen during desktop app execution.
- When `ContentsBodyResourceRenderer` refreshes after a drop/import or same-note reload, the desktop host now reapplies
  the resolved resource payload back into the current RichText editor HTML, keeps the paragraph-sized placeholder slot
  in sync, and schedules a gutter refresh so the source-aligned resource renderer can repaint that body slot without
  waiting for another note-open turn.
- Programmatic RichText surface refresh now also raises `programmaticEditorSurfaceSyncActive` around that host-driven
  repaint window, so the rendered placeholder surface cannot re-enter the typing diff path as a fake user edit while
  resource/body presentation is being rebuilt.
- Desktop file-drop linking now also raises a dedicated `resourceDropEditorSurfaceGuardActive` window around the drop
  turn.
  During that guard:
  - `ContentsInlineFormatEditor.qml` suppresses fallback `textEdited(...)` dispatch from any native RichText
    drop-side mutation that Qt may apply to the visible `TextEdit`
  - `ContentsEditorTypingController.qml` ignores any late edit notification that still escapes that wrapper-level
    suppression
  - after the drop turn settles, the host reapplies the canonical rendered editor surface from `renderedEditorText`
    so the nested `TextEdit` cannot keep a stray literalized `resource`-attribute fragment in its internal buffer
- Desktop now also drives `ContentsInlineFormatEditor.blockExternalDropMutation` from
  `resourceDropActive || resourceDropEditorSurfaceGuardActive`, so the nested `TextEdit` turns read-only from drag
  hover through drop finalization and cannot corrupt adjacent `<callout>` / `<resource>` source by handling the same
  OS file drop as ordinary editable content.
- Desktop now also intercepts `StandardKey.Paste` only while `ResourcesImportViewModel.clipboardImageAvailable` is
  true.
  Image paste therefore reuses the exact same guard + resource-import + `<resource ... />` insertion + deferred reload
  path as drag/drop, while ordinary text clipboard paste still falls through to the native `TextEdit` implementation.
- That same paste path now also reuses the duplicate-name alert flow.
  Repeated `clipboard-image.png` imports therefore require an explicit overwrite/keep-both decision instead of
  silently creating another numbered package.
- Desktop resource cards now also allow the shared bitmap viewer bridge to recover image presentation from the resolved
  asset path/format itself.
  When a renderer payload still reaches QML as `document` but points at a compatible bitmap file, the note body now
  promotes that resource back into the Figma `292:50` image frame.
  The old synthetic `Document Resource` metadata card path is removed entirely, so desktop no longer invents a
  summary tile for unsupported generic `document` entries.
- While that hover-phase resource drop is considered valid, desktop now also calls `drag.acceptProposedAction()`
  before setting `drag.accepted = true`, reducing the chance that the nested `TextEdit` keeps the OS file drag alive
  long enough to fall back to Qt's default image-object insertion path.
- The desktop drop handler now also prefers `drop.acceptProposedAction()` when Qt exposes it, then sets
  `drop.accepted = inserted`, so the file drop is consumed as an editor-import gesture instead of being left for the
  nested `TextEdit` to interpret as ordinary editable content.
- The desktop editor row now reserves right-side space for the minimap rail, but the visible minimap itself is no
  longer a `RowLayout` child.
  The active minimap is now a sibling rail anchored directly to the editor surface's right edge, while the legacy row
  slot is reduced to a zero-width spacer so existing bindings can stay stable.
- That reserved right margin now stays expressed as one parenthesized arithmetic binding, so QML cache compilation
  keeps treating the editor-body inset and the optional minimap-rail width as one margin value instead of splitting
  the rail reservation across statement boundaries.
- That anchored-right rail removes the last dependency on inherited row mirroring or layout-direction state for minimap
  placement.
  Gutter stays on the left, the editor viewport stays in the center, and the minimap rail stays on the right because
  the visible minimap now binds to the editor surface edges instead of row-order heuristics.
- The ordinary structured document viewport is now also restored as a sibling of the print-preview `Flickable`
  instead of being nested underneath it.
  Screen-mode editor flow and the anchored-right minimap rail therefore keep their own parent surface even after the
  print-preview branch is mounted.
- When that structured-flow tail-click lands on a note whose last block is a non-text node, the desktop host now lets
  `ContentsStructuredDocumentFlow.qml` append one trailing newline before restoring focus, so typing can begin
  immediately after an ending resource/divider block.

## Legacy Surface
- The single `ContentsInlineFormatEditor` remains the fallback path for notes without any structured blocks, but the
  inline editor instance is now created only while that fallback path is active.
