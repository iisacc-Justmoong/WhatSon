# `src/app/qml/view/content/editor/ContentsDisplayView.qml`

## Responsibility
Desktop content editor host.

## Editor Input Mode
- Once the note session is bound, desktop now treats `ContentsStructuredDocumentFlow.qml` as the canonical whole-note
  document host.
- `editorSession.editorText` remains the RAW authority, but prose, semantic text blocks, `<resource ... />`,
  `agenda`, `callout`, and `break` now all stay inside that same document host instead of using mode-specific editor
  routing.
- The legacy whole-note `ContentsInlineFormatEditor.qml` path is now transitional fallback only while no bound note
  session exists yet; it is no longer the ordinary note-editing mode.
- When that fallback RichText path does temporarily render an inline image placeholder, the generated `<img>` width now
  uses the full editor-body width budget instead of the older `480px` cap.

## Structured Document Flow
- `agenda`, `callout`, `resource`, `break`, semantic text tags, and plain text now all share the same desktop
  document-flow host.
- Canonical live `<resource ... />` markup is still tracked through the live editor buffer, presentation snapshot, and
  selection-bridge snapshot so `ContentsBodyResourceRenderer` can resolve inline resources against the freshest RAW
  source.
- `ContentsStructuredDocumentFlow.qml` is therefore no longer a special-mode host only for some block families.
  It is the ordinary note-body host for the bound desktop editor session.
- That block stream now also includes parser-owned semantic text blocks for `paragraph` / `title` / `subTitle` /
  `event*` body markup.
  A note that mixes prose paragraphs with `<resource ... />`, `agenda`, `callout`, or `break` therefore keeps all of
  those blocks in one note-owned document flow.
- Structured-flow visibility is now additionally gated by `editorSession.editorBoundNoteId == selectedNoteId`.
  A previously rendered structured note therefore cannot remain mounted after the user selects a different note-list
  item; the host hides stale block content until the newly selected note session is actually bound.
- Structured-flow activation now follows note-session binding itself rather than a special subset of body tags.
  `agenda`, `callout`, `break`, `resource`, and ordinary prose are all treated as ordinary note-body blocks under the
  same host.
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
- That same import path no longer forces a terminal newline when the inserted `<resource ... />` lands at EOF.
  Desktop therefore stops serializing a synthetic trailing `<paragraph></paragraph>` just to park focus below the
  new resource.
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
- Desktop keeps the RichText image-upgrade path only as fallback presentation support for the unloaded legacy editor.
  The bound note session itself now renders inline resource blocks through the canonical document host together with
  text, agenda, callout, and break blocks.
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
- The desktop editable note surface no longer distinguishes “ordinary prose notes” from “agenda/callout/resource
  notes” at the host level.
  All of them now stay in one Apple Notes-like document host backed by `.wsnbody`.
- Desktop gutter and minimap rails now stay mounted for structured-flow notes as well.
  The host no longer treats `<resource ... />` notes as a special full-surface rendering mode that must disable editor
  chrome; instead it asks `ContentsStructuredDocumentFlow.qml` for block-derived logical line entries and feeds those
  lines back into the existing gutter/minimap pipeline.
- The desktop gutter now anchors each structured logical line to that block's real document `contentY`.
  Tall image/divider blocks therefore keep the same vertical offset relationship as the rendered document itself, so
  line numbers after an inline image do not overlap the bitmap while it is still on screen.
- Structured gutter visibility and first-visible-line detection now also read the live structured line-entry list
  directly instead of trusting a separate logical-line-count snapshot.
  A resource-bearing note therefore cannot drop line `1` or top-pack later gutter rows just because one intermediate
  line-count cache or fallback binary search still reflects pre-layout state.
- Structured gutter numbering now also comes from parser/text logical lines first, not from block height heuristics.
  A single wrapped paragraph below an image therefore remains one gutter line unless the authored source actually
  contains another newline-delimited line.
- Structured text-line `y` placement now also comes from delegate-sampled rendered line rectangles where available.
  The gutter no longer keeps an artificial equal-gap rhythm inside one tall text block when the actual text rows sit
  closer to the top or bottom after wrapping.
- Gutter viewport offset now comes from the resolved active editor `Flickable.contentY` for every editor mode, with
  older per-surface offsets kept only as a fallback when no live flickable exists yet.
  Scrolling a note therefore moves gutter line numbers and markers with the document instead of leaving the gutter
  visually pinned while body content continues to scroll underneath it.
- The structured viewport background tap handler now asks `ContentsStructuredDocumentFlow.qml` whether the tap landed
  on a real block before forcing `requestStructuredDocumentEndEdit()`.
  Clicking an inline image/resource block therefore no longer gets immediately overwritten by the viewport's fallback
  tail-focus behavior, and block selection can remain visible.
- Desktop selection context menus now also support parser-owned structured text-block selections.
  When the note is mounted through `ContentsStructuredDocumentFlow.qml`, right-click no longer depends on the unloaded
  legacy whole-note `contentEditor` selection state; the host snapshots the active block-local selection and opens the
  same `LV.ContextMenu` against that structured selection instead.
- While the user is actively typing, desktop now treats gutter refresh as a logical-line-structure concern rather than
  a per-keystroke repaint duty.
  If the latest edit did not add or remove a newline-delimited logical line, line-structure-triggered gutter refresh
  requests are ignored so the visible gutter no longer jitters between stale and recomputed positions on every
  character.
- Desktop no longer keeps a dedicated structured gutter line-entry cache.
  Gutter and structured minimap geometry now read the current `ContentsStructuredDocumentFlow.qml` line entries
  directly at calculation time, so a stale fallback snapshot cannot survive and later overwrite corrected live
  geometry.
- The desktop host now also routes inline-format shortcuts through the active structured block before falling back to
  the legacy whole-editor selection controller.
  Selecting text inside a parser-owned paragraph block and pressing `Bold`, `Highlight`, or the other inline-format
  shortcuts therefore rewrites the owning block's RAW source directly instead of failing because the legacy full-note
  editor is not mounted.
- That structured inline-format bridge now captures the active block index plus selection snapshot and rewrites RAW
  immediately on the shortcut turn itself.
  Desktop formatting shortcuts therefore no longer depend on a delayed follow-up turn that might observe a different
  live focus or selection state.
- The current-line gutter indicator for structured notes now follows the active block's local logical line number, not
  only the first logical line of that block.
  Moving the caret between authored lines inside one paragraph/callout/agenda block therefore moves the blue gutter
  marker to the matching global document line without forcing a whole gutter geometry rebuild.
- The current-line gutter marker now also uses the active structured block's live cursor row rectangle when that block
  can expose one.
  The blue indicator therefore follows the actual visual caret row inside wrapped paragraph/task/callout content
  instead of staying pinned to the top of the owning logical line.
- Resource and divider blocks still count as exactly one visible gutter row.
  The gutter marker height stays at one editor line even when the underlying block is much taller, while viewport
  culling still uses the real block height so the row remains visible throughout the image's on-screen span.
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
- Desktop now also intercepts image paste both through the focused editor key-event path and the window-shortcut
  fallback while `ResourcesImportViewModel.clipboardImageAvailable` is true.
  A `Cmd/Ctrl+V` turn inside the live note editor therefore still reuses the exact same guard + resource-import +
  `<resource ... />` insertion + deferred reload path as drag/drop, while ordinary text clipboard paste still falls
  through to the native `TextEdit` implementation.
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
