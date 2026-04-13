# `src/app/qml/view/content/editor/MobileContentsDisplayView.qml`

## Responsibility
Mobile content editor host.

## Structured Document Flow
- `agenda`, `callout`, and `break` remain `.wsnbody` body tags and no longer auto-promote the note into a separate
  structured editor surface.
- Mobile currently keeps `structuredDocumentFlowEnabled: false`, so shortcut insertion and note-open stay on the
  legacy single-editor/body-overlay path even when `ContentsStructuredBlockRenderer` detects structured tags.
- `ContentsStructuredDocumentFlow.qml` remains in the tree as an experimental surface, but it is not the default note
  editing presentation.
- Structured block rewrites route through `applyDocumentSourceMutation(...)` so mobile keeps the same RAW persistence
  contract as desktop, but they no longer force an immediate full legacy presentation rebuild while structured-flow
  editing remains active.
- While structured-flow mode is active, mobile also disconnects the hidden legacy agenda/callout overlay models so note
  open does not pay for both rendering paths at once.
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
- During a note-selection transition, the mobile host now projects any still-live `TextEdit` delta through
  `ContentsEditorTypingController.handleEditorTextEdited()` before deciding whether to flush the previously bound note.
  This keeps large deletions or other last-turn edits from being dropped just because the selection id changed before
  the session buffer had caught up.
- Mobile inline-editor `onTextEdited()` now only notifies the typing controller to read the live `TextEdit` state.
  The host no longer treats the rendered RichText surface payload itself as a recovery source for RAW note text, so
  agenda/callout projection cannot push the note-open session snapshot back over newer visible edits.
- Mobile host-side RAW mutations such as imported-resource tag insertion or structured source rewrites now also issue
  immediate persistence requests directly; native-input preference no longer implies a deferred-persistence exception.
- Mobile editor drop handling now also accepts native file-manager drags without a `DropArea.keys` MIME gate, parses
  both `drop.urls` and `text/uri-list`, imports those files through `ResourcesImportViewModel`, injects canonical
  `<resource ...>` calls into the active note source, and feeds the current presentation snapshot into
  `ContentsBodyResourceRenderer` so the dropped resource card appears in the body overlay before the worker-thread note
  flush finishes.
- The post-import insertion path now normalizes `importUrlsForEditor(...)` results from either a real JS array or a
  Qt-provided list-like `QVariantList`, so successful hub imports cannot silently skip body-tag insertion just because
  the invokable return value is not tagged as `Array.isArray(...)` in QML.
- Mobile resource-drop insertion now also writes canonical self-closing `<resource ... />` RAW source with quoted,
  XML-escaped attributes.
- Mobile no longer inserts those resource tags by treating the visible editor cursor as a direct `.wsnbody` source
  offset. The drop path now reuses `ContentsEditorTypingController.insertRawSourceTextAtCursor(...)` so logical caret
  positions are translated back into RAW source offsets before save.
- Mobile now also normalizes that inserted resource block onto standalone source lines when the drop happens inside an
  existing paragraph, keeping the inline resource slot block-owned instead of sharing one prose line with adjacent
  text.
- Mobile now likewise defers the resources runtime reload until after the drop turn finishes its same-note
  `<resource ... />` link attempt, keeping `.wsresource` package creation and `.wsnbody` linking on one stable editor
  turn while still refreshing the runtime for successful package registration.
- Mobile resource rendering now also uses `ContentsResourceLayer.qml` instead of the old bottom overlay stack.
  The plain-text logical projection reserves a fixed blank block for each resource tag, and the inline resource frame
  is anchored back onto that authored source position so the note body itself owns the rendered media slot.
- When the selection bridge can already expose a buffered dirty body for the newly selected note, the mobile host now
  consumes that note-owned payload through the ordinary selection-sync path instead of waiting for a stale filesystem
  read to arrive first.
- Mobile note-open now also keeps the legacy inline editor path alive until the first settled structured render confirms
  that the current selected note actually owns agenda/callout/break blocks.
- Once structured mode has activated for that same note, later async reparses keep the structured surface mounted
  instead of bouncing through the legacy editor again.
- Mobile note snapshot polling also pauses during `selectedNoteBodyLoading`, so the first lazy note-open read is not
  interleaved with a second same-note refresh probe.
- Mobile no longer auto-mounts `ContentsStructuredTagValidator` as a parser-driven direct-write path.
  Structured note open and editing therefore stay on the same session persistence line as desktop instead of adding a
  second validator-owned file update turn.
- The print-document `Repeater` delegate now declares `required property int index`, and the inline editor host uses a
  literal `shapeStyle: 0`, removing runtime `ReferenceError` noise seen during live mobile-host execution.
