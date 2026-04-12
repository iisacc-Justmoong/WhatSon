# `src/app/qml/view/content/editor/ContentsDisplayView.qml`

## Responsibility
Desktop content editor host.

## Structured Document Flow
- `agenda`, `callout`, and `break` remain `.wsnbody` body tags and no longer auto-promote the note into a separate
  structured editor surface.
- Desktop currently keeps `structuredDocumentFlowEnabled: false`, so shortcut insertion and note-open stay on the
  legacy single-editor/body-overlay path even when `ContentsStructuredBlockRenderer` detects structured tags.
- `ContentsStructuredDocumentFlow.qml` remains in the tree as an experimental surface, but it is not the default note
  editing presentation.
- Source persistence for block edits now runs through `applyDocumentSourceMutation(...)`, which updates the RAW body,
  marks local authority, optionally restores focus inside the reparsed block, and only forces a full legacy
  presentation rebuild when the note actually falls back out of structured-flow mode.
- While structured-flow mode is active, legacy agenda/callout overlay layers now receive empty models so hidden fallback
  delegates do not instantiate in parallel with the document-native block flow.
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
- Desktop host-side RAW mutations such as imported-resource tag insertion or structured source rewrites now also issue
  immediate persistence requests directly; the host no longer keeps a local deferred-persistence override for those
  editor mutations.
- When the selection bridge can already expose a buffered dirty body for the newly selected note, the desktop host now
  consumes that note-owned payload through the ordinary selection-sync path instead of waiting for a stale filesystem
  read to arrive first.
- While structured-flow mode is active, the legacy `ContentsInlineFormatEditor` now unloads entirely through a `Loader`
  instead of remaining alive behind `visible: false`.
- The host keeps a lightweight proxy object under the existing `contentEditor` reference so shared geometry/focus helpers
  can keep null-safe access patterns even while the legacy editor instance is absent.
- Desktop note-open now keeps the legacy inline editor path alive until the first settled structured render confirms
  that the currently selected note actually owns agenda/callout/break blocks.
- After that first same-note activation, later async reparses keep the structured-flow surface mounted instead of
  bouncing back through the legacy editor during in-block editing.
- Timer-driven note snapshot polling now also pauses while the selected note body is still loading, so note-open does
  not compete with an overlapping same-note refresh probe.
- Desktop no longer auto-mounts `ContentsStructuredTagValidator` as a parser-driven write path.
  Renderer-side correction suggestions may still exist internally, but note-open and typing now stay on the single
  editor-session persistence path instead of opening an extra validator-triggered file write + note-list refresh turn.

## Legacy Surface
- The single `ContentsInlineFormatEditor` and overlay layers still remain the fallback path for notes without any
  structured blocks, but the inline editor instance is now created only while that fallback path is active.
