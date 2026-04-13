# `src/app/qml/view/content/editor/ContentsDisplayView.qml`

## Responsibility
Desktop content editor host.

## Structured Document Flow
- `agenda`, `callout`, `resource`, and `break` remain `.wsnbody` body tags, and desktop now promotes the note into the
  structured document-flow surface as soon as canonical body-level resource markup is present in the active source.
- Desktop now treats a canonical live `<resource ... />` tag in either the live editor buffer or the current
  presentation snapshot as an immediate structured-flow activation request.
  `bodyResourceRenderer.resourceCount` still keeps already-resolved notes active, but the host no longer waits for a
  later resource-resolution turn before leaving the legacy single-editor path.
- `ContentsStructuredDocumentFlow.qml` is therefore no longer dormant: notes with inline resource tags now render
  those resources as body-owned QML blocks instead of RichText placeholders or offset overlays.
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
- The dedicated full-surface `ContentsResourceViewer` is now reserved for direct resource-package browsing from the
  Resources hierarchy only.
  A note opened from Library, Bookmarks, Projects, Tags, Progress, Event, or Preset must stay on the ordinary note
  editor surface even if its `.wsnbody` contains inline `<resource ... />` blocks.
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
- During a note-selection transition, the desktop host now projects any still-live `TextEdit` delta through
  `ContentsEditorTypingController.handleEditorTextEdited()` before deciding whether to flush the previously bound note.
  This keeps large deletions or other last-turn edits from being dropped just because the selection id changed before
  the session buffer had caught up.
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
- Desktop now also rescans the live `editorText` buffer for canonical `<resource ... />` markup on each edit turn.
  Dropping an image into a note that already contains paragraphs therefore switches that same note into the structured
  resource path immediately instead of leaving a legacy RichText/plain-text frame alive long enough to expose the raw
  tag as visible prose.
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
- Desktop no longer splices that imported resource tag block by the raw `TextEdit.cursorPosition` integer. Instead it
  routes the RAW insertion through `ContentsEditorTypingController.insertRawSourceTextAtCursor(...)`, so logical/plain
  caret positions are mapped back into `.wsnbody` source offsets before persistence.
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
- `ContentsBodyResourceRenderer` now follows the live `editorText` buffer not only after structured-flow is already
  visible, but also during the activation turn where a canonical live `<resource ... />` tag first appears.
  Mixed prose + image notes therefore resolve the actual `.wsresource` payload from the same RAW source that just
  received the drop insertion, instead of waiting for one more presentation snapshot cycle.
- When `ContentsBodyResourceRenderer` resolves an inline image resource successfully, the placeholder block is now
  rewritten into a real RichText `<img>` paragraph so the bitmap becomes part of the editor's own document flow rather
  than only a body-aligned overlay.
- The RichText-side image rewrite now prefers the renderer's resolved resource URL and no longer appends extra blank
  placeholder paragraphs after successful inline image injection.
- `ContentsBodyResourceRenderer` now also receives `libraryHierarchyViewModel` as a fallback note-directory resolver,
  so note-body resource rendering survives hierarchy-switch lag or active domains that do not implement
  `noteDirectoryPathForNoteId(QString)`.
- RichText dirtiness checks now compare against the already-upgraded inline-resource HTML, so the desktop host no
  longer treats every resource-bearing note as permanently dirty just because `editorSurfaceHtml` still contains the
  placeholder marker payload.
- `ContentsResourceLayer.qml` now remains only for resource types that are not upgraded into RichText-inline media
  blocks yet, or for native/plain-input routes where the host is not using the RichText editor surface projection.
  Desktop non-image or unresolved resources still use that layer, while resolved bitmap images now stay in the
  RichText document body.
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
- Desktop note-open now keeps the legacy inline editor path alive until the first settled structured render confirms
  that the currently selected note actually owns agenda/callout/break blocks.
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
- While that hover-phase resource drop is considered valid, desktop now also calls `drag.acceptProposedAction()`
  before setting `drag.accepted = true`, reducing the chance that the nested `TextEdit` keeps the OS file drag alive
  long enough to fall back to Qt's default image-object insertion path.
- The desktop drop handler now also prefers `drop.acceptProposedAction()` when Qt exposes it, then sets
  `drop.accepted = inserted`, so the file drop is consumed as an editor-import gesture instead of being left for the
  nested `TextEdit` to interpret as ordinary editable content.
- The desktop minimap slot is now also pinned to a fixed layout width while visible.
  The right-side minimap therefore keeps its column even when the central editor viewport expands aggressively inside
  the shared `RowLayout`.
- The desktop editor `RowLayout` now also declares `layoutDirection: Qt.LeftToRight` explicitly and disables
  `LayoutMirroring` inheritance on that row.
  Gutter -> editor viewport -> minimap ordering therefore stays stable even if ambient layout mirroring or inherited
  direction settings change elsewhere in the shell.
- The minimap delegate itself now also declares `Layout.alignment: Qt.AlignRight | Qt.AlignTop`, reinforcing that the
  fixed-width minimap rail belongs to the rightmost editor column.
- When that structured-flow tail-click lands on a note whose last block is a non-text node, the desktop host now lets
  `ContentsStructuredDocumentFlow.qml` append one trailing newline before restoring focus, so typing can begin
  immediately after an ending resource/divider block.

## Legacy Surface
- The single `ContentsInlineFormatEditor` and overlay layers still remain the fallback path for notes without any
  structured blocks, but the inline editor instance is now created only while that fallback path is active.
