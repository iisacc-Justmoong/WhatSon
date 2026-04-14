# `src/app/qml/view/content/editor/MobileContentsDisplayView.qml`

## Responsibility
Mobile content editor host.

## Editor Input Mode
- The mobile note editor now keeps its mounted `ContentsInlineFormatEditor.qml` on the plain-text input path while the
  renderer layer continues to receive the latest RAW source for HTML conversion on each presentation refresh turn.
  The editable `TextEdit` itself is therefore no longer asked to host RichText document state.

## Structured Document Flow
- `agenda`, `callout`, `resource`, and `break` remain `.wsnbody` body tags, and mobile now lets any parsed explicit
  document block, including semantic text-tag blocks and `resource`, activate `ContentsStructuredDocumentFlow.qml`.
  A note whose RAW body contains `<resource ... />` therefore renders that block through parser-owned document flow
  instead of depending on the legacy editor overlay path.
- Canonical live `<resource ... />` markup is still tracked through the live editor buffer, presentation snapshot, and
  selection-bridge body-source snapshot so `ContentsBodyResourceRenderer` can resolve against the freshest RAW source.
- `ContentsStructuredDocumentFlow.qml` therefore becomes the shared block-flow host for agenda/callout/break/resource
  notes and for notes whose RAW still carries explicit semantic text tags whenever the parser reports explicit
  document blocks.
- That block-flow stream now also includes parser-owned semantic text blocks for `paragraph` / `title` / `subTitle` /
  `event*` body markup.
  Mobile notes that mix prose with `<resource ... />` tags therefore keep their authored text visible in the same
  document flow as the inline image frame instead of collapsing into a resource-only surface.
- Structured-flow visibility is now additionally gated by `editorSession.editorBoundNoteId == selectedNoteId`.
  A previously rendered structured note therefore cannot remain mounted after the user selects a different note-list
  item; the host hides stale block content until the newly selected note session is actually bound.
- Structured-flow activation is now also gated by `structuredBlockRenderer.hasRenderedBlocks` only.
  A live `<resource ... />` token alone therefore cannot replace the mobile editor with an early image-only block
  surface before the parser has emitted the full document block sequence.
- Structured block rewrites route through `applyDocumentSourceMutation(...)` so mobile keeps the same RAW persistence
  contract as desktop, but they no longer force an immediate full legacy presentation rebuild while structured-flow
  editing remains active.
- While structured-flow mode is active, mobile also disconnects the hidden legacy agenda/callout overlay models so note
  open does not pay for both rendering paths at once.
- The same structured-flow surface now also receives `bodyResourceRenderer.renderedResources`, letting mobile resource
  blocks resolve from `<resource ... />` to the actual asset file inside the referenced `.wsresource` package before
  `ContentsResourceRenderCard` paints the inline frame.
- In screen editor mode, that structured-flow column now uses the same effective body width contract as ordinary note
  text: viewport width minus the editor's left/right body inset.
  Inline image resources therefore fill the note body column itself rather than the entire editor viewport.
- Mobile host no longer mounts a dedicated `ContentsResourceViewer` surface for note editing.
  Inline `<resource ... />` tags must stay inside `ContentsStructuredDocumentFlow.qml` as ordinary document blocks,
  and note hosts no longer switch to a resource-only surface or resource overlay layer.
- While structured-flow mode is active, a mobile left-tap in the structured document viewport now routes through
  `requestStructuredDocumentEndEdit()` so inline resource notes can always reopen an editable tail position.
- The legacy mobile `ContentsInlineFormatEditor` now unloads entirely during structured-flow editing and is recreated only
  when the fallback plain-text path becomes active again.
- Mobile keeps a lightweight proxy object behind the shared `contentEditor` reference so existing geometry/focus helpers
  remain null-safe while the legacy editor instance is absent.
- Mobile host-side selection and typing controllers now bind to `contentsView.contentEditor` explicitly instead of
  relying on a self-referential `contentEditor: contentEditor` assignment that produced runtime binding loops under
  bound-component semantics.
- Mobile note-open reconcile now also uses a deferred request/complete cycle:
  - selection changes queue one reconcile attempt per note
  - `selectionBridge.viewSessionSnapshotReconciled(...)` closes that pending state
  - timer-driven snapshot polling no longer performs synchronous RAW note reads on the UI thread
- Mobile polling now also skips overlapping reconcile requests for the same selected note while one worker fetch is
  already pending.
- Mobile body-sync and reconcile refresh work now also routes through `editorSession.editorTextSynchronized`, removing
  duplicate minimap/presentation/gutter refresh scheduling from multiple completion handlers.
- Mobile note entry now also resets the shared gutter-line model and line-geometry cache as soon as the selected note
  changes, then rebuilds them again when `editorSession.editorTextSynchronized` binds the new note body.
  Even though the current mobile shell does not expose the gutter rail, the shared line-geometry state now refreshes
  per note entry instead of carrying the previous note's cached positions until another incidental layout event.
- Mobile selection-driven editor sync now also collapses initial mount, `selectedNoteIdChanged`, and
  `selectedNoteBodyTextChanged` into one queued `scheduleSelectionModelSync(...)` pass per event-loop turn, and mobile
  visibility re-entry now reuses that same helper instead of scheduling a parallel note-open refresh path. One
  note-open transition therefore no longer replays the same editor/session refresh logic twice from separate handlers.
- The shared mobile selection-sync helper also keeps the `requestSyncEditorTextFromSelection(...)` result in one local
  gate before fallback refresh scheduling, matching the desktop deferred sync path while staying within QML parser
  identifier constraints.
- Mobile note-open now also waits for the selected note body lazy-load to finish before pushing selection state into
  the live editor session.
  While loading is pending, mobile disables the editor viewport, keeps the previous buffer intact, can request an
  immediate fetch attempt for the previously bound note, and shows the same loading overlay contract as desktop instead
  of syncing an empty placeholder body.
- Mobile note-open now also re-queues `scheduleSelectionModelSync(...)` when
  `selectedNoteBodyLoading` returns to `false`, even if the selected note still resolves to an empty string body.
  Empty-body notes therefore clear the stale previous note session instead of silently inheriting the last visible
  editor contents.
- Mobile also now requires `selectedNoteBodyNoteId == selectedNoteId` before syncing selection state into the session
  or restoring editor focus, so a stale body payload cannot be rebound under a different note id.
- During a note-selection transition, the mobile host no longer asks
  `ContentsEditorTypingController.handleEditorTextEdited()` to diff the editor surface after `selectedNoteId` has
  already changed.
  Once the bound session note and selected note diverge, the host waits for same-note rebinding before any further
  diff/persist work, preventing one note's stale editor surface from being serialized into another note.
- That deferred blur-side flush now also captures the note id that owned the editor when focus was lost and refuses to
  run after the session has rebound to a different selected note.
  Tapping from note `A` to note `B` therefore cannot reinterpret `A`'s stale editor surface as a late edit for `B`
  and overwrite the newly selected note body during the transition.
- That same blur-side flush now also requires an active local authoring session or a pending body-save on the same
  bound note before it runs.
  Mobile note-list selection alone therefore cannot escalate an untouched selected note into a persistence turn just
  because the editor lost focus.
- Timer-driven snapshot polling and one-shot entry reconcile now also require both of these same-note ownership checks
  before sending `editorSession.editorText` to the selection bridge:
  - `editorSession.editorBoundNoteId == selectedNoteId`
  - `selectionBridge.selectedNoteBodyNoteId == selectedNoteId`
  A newly selected note therefore cannot be reconciled against the previously bound note's stale session text.
- Mobile inline-editor `onTextEdited()` now only notifies the typing controller to read the live `TextEdit` state.
  The host no longer treats the rendered RichText surface payload itself as a recovery source for RAW note text, so
  agenda/callout projection cannot push the note-open session snapshot back over newer visible edits.
- Mobile now likewise routes Backspace/Delete through `ContentsEditorTypingController.handleTagAwareDeleteKeyPress(...)`
  before the generic shortcut controller and before `TextEdit` default deletion.
  When the source cursor touches a proprietary tag token, mobile now removes that whole token in one RAW mutation.
- Mobile host-side RAW mutations such as imported-resource tag insertion or structured source rewrites now also issue
  immediate persistence requests directly; native-input preference no longer implies a deferred-persistence exception.
- Mobile editor drop handling now also accepts native file-manager drags without a `DropArea.keys` MIME gate, parses
  `drop.urls` first and then falls back through string-based payloads such as `drop.text`, `text/uri-list`,
  `text/plain`, and platform file-url MIME values, imports those files through `ResourcesImportViewModel`, injects
  canonical `<resource ...>` calls into the active note source, and feeds the current presentation snapshot into
  `ContentsBodyResourceRenderer` so the dropped resource card appears in the body overlay before the worker-thread note
  flush finishes.
- Before that import actually runs, mobile now asks `ResourcesImportViewModel.inspectImportConflictForUrls(...)`
  or `inspectClipboardImageImportConflict()` whether the incoming asset name already exists.
  If it does, mobile opens the same `LV.Alert` decision surface as desktop with `Overwrite`, `Keep Both`, and
  `Cancel Import` actions instead of silently auto-numbering the duplicate.
- Mobile now also rescans the live `editorText` buffer for canonical `<resource ... />` markup on each edit turn.
  That keeps resource resolution tied to the newest RAW source while still allowing parser-owned resource blocks to
  activate the structured document flow on the same selected note.
- The mobile host now also exposes the same canonical resource-tag counting/loss-detection helpers as desktop.
  That loss check compares candidate rewrites against the strongest currently known RAW baseline
  (`editorText`, `documentPresentationSourceText`, and the selected-note body snapshot only when that snapshot still
  belongs to the selected note and the host has not taken local editor authority), so stale legacy-surface rewrites
  cannot erase `<resource ... />` tokens from `.wsnbody` just because one buffer had already drifted without letting an
  older model snapshot override a newly accepted local edit.
- `applyDocumentSourceMutation(...)` now also runs that same loss check before it mutates `editorText` or persists.
  Any mobile caller that bypasses the typing/selection controllers therefore still cannot silently delete
  `<resource ... />` from RAW while the host remains on the legacy inline-editor surface.
- Figma node `294:7933` is also the mixed-content reference for mobile-hosted note bodies:
  text and image frames share one authored document column, so inline images must not replace the entire editor with a
  dedicated resource viewer unless the user is directly browsing a resource package from the Resources hierarchy.
- That same reference now also governs the shared body rhythm:
  - mixed prose/image blocks keep visible spacing between siblings
  - paragraph text keeps the denser 12px medium-weight note-body feel instead of a tall field-like minimum height
- The post-import insertion path now normalizes `importUrlsForEditor(...)` results from either a real JS array or a
  Qt-provided list-like `QVariantList`, so successful hub imports cannot silently skip body-tag insertion just because
  the invokable return value is not tagged as `Array.isArray(...)` in QML.
- Mobile resource-drop insertion now also writes canonical self-closing `<resource ... />` RAW source with quoted,
  XML-escaped attributes.
- Mobile no longer inserts those resource tags by treating the visible editor cursor as a direct `.wsnbody` source
  offset.
  On the legacy inline-editor path the drop handler reuses
  `ContentsEditorTypingController.insertRawSourceTextAtCursor(...)` so logical caret positions are translated back
  into RAW source offsets before save.
- Mobile now also routes plain `Enter` / `Return` through
  `ContentsEditorTypingController.handlePlainEnterKeyPress(...)` before falling back to delete/formatting shortcuts.
  The legacy inline editor therefore splits lines by mutating RAW source directly instead of trusting a transient
  RichText paragraph rewrite.
- When the note is already inside `ContentsStructuredDocumentFlow.qml` because of agenda/callout/break blocks, mobile
  now inserts dropped resource tags through the structured-flow host's active-block insertion path instead of falling
  back to the legacy inline-editor cursor bridge.
- Mobile now also normalizes that inserted resource block onto standalone source lines when the drop happens inside an
  existing paragraph, keeping the inline resource slot block-owned instead of sharing one prose line with adjacent
  text.
- Mobile now likewise defers the resources runtime reload until after the drop turn finishes its same-note
  `<resource ... />` link attempt, keeping `.wsresource` package creation and `.wsnbody` linking on one stable editor
  turn while still refreshing the runtime for successful package registration.
- Mobile now also carries the same `whatson-resource-block` upgrade helper as desktop.
- Once the selected note has entered structured-flow mode, mobile now prefers `ContentsResourceBlock.qml` in the shared
  structured document host for inline resource display instead of depending on the RichText placeholder upgrade path.
- `ContentsBodyResourceRenderer`, `ContentsStructuredBlockRenderer`, and `ContentsStructuredDocumentFlow` now share one
  `structuredFlowSourceText` contract.
  Mobile resource-block activation therefore stays aligned even when the live editor surface, deferred presentation
  snapshot, and selection-bridge source are briefly out of step.
- Mobile now also gates `ContentsBodyResourceRenderer.bodySourceText` by
  `editorSession.editorBoundNoteId == selectedNoteId` instead of by the selection-bridge body-note echo alone.
  A same-note drag/drop or clipboard image insert therefore resolves the newly inserted `<resource ... />` tag from
  the live bound editor RAW immediately, even before the saved-body snapshot finishes catching up.
- Mobile now also binds `ContentsBodyResourceRenderer.noteDirectoryPath` from
  `selectionBridge.selectedNoteDirectoryPath`.
  Inline resource rendering therefore reuses the mounted editor session's resolved note package path instead of
  depending only on the currently active hierarchy view-model resolver.
- The current default mobile editable note surface also keeps RichText inline-image upgrading disabled.
  Resolved bitmap resources therefore render only through parser-owned document blocks inside
  `ContentsStructuredDocumentFlow.qml`, keeping authored text and image frames in one flow without a second overlay
  surface.
- Mobile note transitions now also keep structured-flow notes off the legacy whole-editor typing diff path.
  Leaving a note whose body contains parser-owned `resource` blocks therefore no longer lets the fallback inline-editor
  serializer rewrite or damage `<resource ... />` source during transition cleanup.
  The same cleanup path now also aborts when the legacy inline editor is not actually mounted, so selection changes
  cannot diff against the proxy editor object and accidentally rewrite the previously bound note source.
- `ContentsBodyResourceRenderer` on mobile now also receives `libraryHierarchyViewModel` as a fallback note-directory
  resolver so body resource rendering survives hierarchy-switch lag and active domains that do not expose
  `noteDirectoryPathForNoteId(QString)`.
- RichText dirtiness checks now compare against the already-upgraded inline-resource HTML instead of the unresolved
  placeholder payload, preventing resource-bearing notes from staying permanently dirty while the RichText surface is
  active.
- Mobile host no longer uses `ContentsResourceLayer.qml` for note-body rendering.
  Resource visibility now depends on parsed document blocks, not on an overlay pass above the editor surface.
- When the selection bridge can already expose a buffered dirty body for the newly selected note, the mobile host now
  consumes that note-owned payload through the ordinary selection-sync path instead of waiting for a stale filesystem
  read to arrive first.
- Mobile note-open now also keeps the legacy inline editor path alive only until the first settled structured render confirms
  that the current selected note actually owns parsed explicit document blocks such as agenda/callout/break/resource or
  semantic text-tag blocks.
- Once structured mode has activated for that same note, later async reparses keep the structured surface mounted
  instead of bouncing through the legacy editor again.
- Mobile note snapshot polling also pauses during `selectedNoteBodyLoading`, so the first lazy note-open read is not
  interleaved with a second same-note refresh probe.
- Mobile no longer auto-mounts `ContentsStructuredTagValidator` as a parser-driven direct-write path.
  Structured note open and editing therefore stay on the same session persistence line as desktop instead of adding a
  second validator-owned file update turn.
- The print-document `Repeater` delegate now declares `required property int index`, and the inline editor host uses a
  literal `shapeStyle: 0`, removing runtime `ReferenceError` noise seen during live mobile-host execution.
- When `ContentsBodyResourceRenderer` refreshes after import or same-note reload, mobile now reapplies that resource
  payload back into the current RichText editor HTML, keeps the placeholder body slot in sync, and schedules a gutter
  refresh so the source-aligned resource renderer can repaint that slot without waiting for another note-open turn.
- Programmatic RichText surface refresh now also raises `programmaticEditorSurfaceSyncActive` around that host-driven
  repaint window, so the rebuilt placeholder surface cannot re-enter the typing diff path as a fake committed edit.
- Mobile file-drop linking now also raises `resourceDropEditorSurfaceGuardActive` for the drop turn itself.
  That guard suppresses wrapper-level fallback `textEdited(...)` dispatch, tells
  `ContentsEditorTypingController.qml` to ignore any same-turn late surface edit notification, and finally reapplies
  the canonical rendered editor surface so native `TextEdit` drop mutations cannot survive as literalized
  `resource`-attribute fragments in the editor buffer.
- Mobile now also binds `StandardKey.Paste` to the same resource-import path while the clipboard contains image data.
  Hardware-keyboard image paste therefore lands in `.wsresource` packaging plus canonical `<resource ... />` RAW
  insertion, while non-image clipboard paste still stays on the ordinary native text-input path.
- That same paste path now also routes through the duplicate-name alert flow.
  Repeated `clipboard-image.png` imports therefore require an explicit overwrite/keep-both decision instead of
  silently creating another numbered package.
- Mobile resource cards now also let the shared bitmap viewer recover image presentation from the resolved asset
  path/format itself.
  A resource that arrives as `document` in the renderer payload but still resolves to a compatible bitmap file now
  promotes back into the Figma `292:50` image frame.
  The old synthetic `Document Resource` metadata card path is removed entirely on mobile as well, so unsupported
  generic `document` entries no longer fabricate a summary tile.
- The shared editor row now reserves right-side space for a minimap rail, but the visible minimap itself is no longer
  hosted inside that `RowLayout`.
  A sibling rail is anchored directly to the editor surface's right edge, while the old row slot is reduced to a
  zero-width spacer so shared bindings remain stable.
- That reserved right margin now also stays as one parenthesized arithmetic binding, so QML cache compilation keeps
  the editor-body inset and the optional minimap width in one margin expression instead of parsing the rail
  reservation as separate statements.
- This matches the desktop contract and removes minimap placement from row-order or mirroring side effects.
  If mobile ever enables the rail again, gutter/editor/minimap ordering will still resolve left/center/right from
  surface-edge anchors instead of inherited row layout state.
- The ordinary structured document viewport is now also kept as a sibling of the print-preview `Flickable` rather than
  a child below that print branch.
  Mobile screen-mode flow and the anchored minimap rail therefore keep their own stable editor-surface parent.
- If the current structured note ends with a non-text block, the mobile host now also allows
  `ContentsStructuredDocumentFlow.qml` to append one trailing newline before focus restore, so typing can resume
  directly after a terminal resource/divider block.
- Mobile now also drives `ContentsInlineFormatEditor.blockExternalDropMutation` from
  `resourceDropActive || resourceDropEditorSurfaceGuardActive`, so the nested `TextEdit` turns read-only as soon as a
  valid external file drag is hovering over the editor and stays frozen until the dedicated resource-drop turn
  finishes.
- While that hover-phase file drag is valid, mobile now also prefers `drag.acceptProposedAction()` before marking the
  drag accepted, so the nested editor does not get to keep the platform file drop on Qt's default content-insertion
  path.
- Mobile drop handling now also prefers `drop.acceptProposedAction()` when the Qt drop event exposes it, then sets
  `drop.accepted = inserted`, keeping file import/linking on the dedicated resource-drop path instead of leaving the
  nested editor free to reinterpret the drop as editable text content.
