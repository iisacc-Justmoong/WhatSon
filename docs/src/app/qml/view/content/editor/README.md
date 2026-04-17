# `src/app/qml/view/content/editor`

## Status
- Directory mirror generated from the current `src` tree.
- This file is the entry point for the detailed documentation pass of this directory.

## Scope
- Mirrored source directory: `src/app/qml/view/content/editor`
- Child directories: 0
- Child files: 30

## Child Directories
- No child directories.

## Child Files
- `ContentsAgendaLayer.qml`
- `ContentsAgendaBlock.qml`
- `ContentsBreakBlock.qml`
- `ContentsCalloutLayer.qml`
- `ContentsCalloutBlock.qml`
- `ContentsDisplayHostModePolicy.qml`
- `ContentsDisplayView.qml`
- `ContentsDocumentBlock.qml`
- `ContentsDocumentTextBlock.qml`
- `ContentsEditorDebugTrace.js`
- `ContentsEditorSelectionController.qml`
- `ContentsEditorSession.qml`
- `ContentsEditorSurfaceGuardController.qml`
- `ContentsEditorTypingController.qml`
- `ContentsGutterLayer.qml`
- `ContentsImageResourceFrame.qml`
- `ContentsInlineFormatEditor.qml`
- `ContentsInlineResourcePresentationController.qml`
- `ContentsMinimapLayer.qml`
- `ContentsMinimapSnapshotSupport.js`
- `ContentsResourceBlock.qml`
- `ContentsResourceViewer.qml`
- `ContentsResourceRenderCard.qml`
- `ContentsResourceDropPayloadParser.qml`
- `ContentsResourceImportConflictController.qml`
- `ContentsResourceImportController.qml`
- `ContentsResourceLayer.qml`
- `ContentsResourceTagController.qml`
- `ContentsStructuredCursorSupport.js`
- `ContentsStructuredDocumentFlow.qml`

## Current Notes

- `ContentsDisplayView.qml` is now the unified desktop/mobile editor surface.
- `ContentsDisplayHostModePolicy.qml` now owns platform display deltas such as gutter/minimap visibility, editor
  horizontal inset, native-input autofocus, and font weight so the shared host no longer forks into separate
  desktop/mobile QML roots.
- The shared live editor engine is already `QtQuick.TextEdit` wrapped by `ContentsInlineFormatEditor.qml`; this
  directory no longer depends on an LVRS `LV.TextEditor` implementation.
- The editor, gutter, and minimap fill the `ContentsView` slot; note-body resources no longer use a separate overlay
  host above the editor.
- `ContentsResourceRenderCard.qml` now centralizes the shared desktop/mobile inline resource card used inside parsed
  note-body blocks, so file/image/audio/pdf/document presentation no longer duplicates the same card scaffolding in
  both hosts.
- `ContentsResourceImportController.qml` is now only the public coordinator for editor-side resource import.
  Drag/drop payload parsing, duplicate-import prompt state, RAW tag insertion, inline HTML presentation, and
  editor-surface guard state now live in dedicated sibling helpers instead of a single import god object.
  The controller no longer receives the full host `view`; desktop/mobile hosts now pass only the explicit callbacks,
  flags, projection object, and import-policy contracts each helper actually needs.
- Inline image resources now compose through `ContentsImageResourceFrame.qml` as transparent border-only cards whose
  runtime outer width follows the editor block width, whose inner bitmap viewport stays centered at natural size until
  the body column forces it smaller, and whose inline media height is capped to the Figma note-block budget so tall
  images remain ordinary document blocks instead of taking over the whole editor column.
- `ContentsResourceViewer.qml` remains the low-level bitmap/PDF viewport component used by resource cards, but note
  hosts no longer swap the whole editor surface into a dedicated resource viewer.
- Resource-bearing note bodies now activate `ContentsStructuredDocumentFlow.qml` so `<resource ... />` stays in the
  same authored block stream as surrounding text.
- `ContentsStructuredDocumentFlow.qml` now mounts one generic `ContentsDocumentBlock.qml` delegate per parsed block
  instead of branching directly to block-type-specific delegates in the repeater host.
- Parser payloads and mounted block delegates now share one block contract:
  `plainText`, `textEditable`, `atomicBlock`, `gutterCollapsed`, `logicalLineCountHint`,
  `minimapVisualKind`, and `minimapRepresentativeCharCount`.
  Gutter, minimap, current-line focus, and nearest-editable-block resolution now consume that shared contract rather
  than branching on block type names inside the flow host.
- Resource blocks still advertise themselves to the minimap as block silhouettes, but the structured-flow cache now
  caps one resource block to ten minimap rows so tall inline media do not overwhelm the rail.
- `ContentsDocumentTextBlock.qml` no longer uses an HTML overlay for inline formatting preview.
  Structured paragraph editing now happens directly against RAW block source text.
- `ContentsResourceBlock.qml` no longer keeps a resource-local `before/selected/after` boundary-editor state machine.
  Resource rows now behave as plain atomic document blocks that emit selection, deletion, and boundary-navigation
  requests back to the flow host.
- Block-boundary keyboard navigation is now being consolidated under `ContentsStructuredDocumentFlow.qml` itself.
  Text/resource/break/callout/agenda delegates are expected to emit generic boundary-navigation requests and let the
  flow resolve the immediately adjacent parsed block from the `.wsnbody` stream rather than hardcoding neighbor lookup
  rules inside each block widget.
- `ContentsEditorTypingController.qml` now owns ordinary text-entry mutation routing so typing no longer reserializes
  the whole presentation overlay on every edit.
- The editor directory now follows one write direction for live note editing:
  RAW `.wsnote/.wsnbody` source is the only write authority, parsers/builders derive presentation state from that RAW
  source, and HTML/DOM presentation surfaces are no longer allowed to serialize themselves back into stored source
  during ordinary typing.
- Desktop/mobile editor views now keep a separate presentation timer for whole-document markdown/HTML projection refresh, so
  `ContentsTextFormatRenderer` and full minimap resampling no longer run directly on every committed keystroke.
- Desktop/mobile editor views now also keep `documentPresentationSourceText` as the single whole-document presentation
  snapshot. One `ContentsEditorPresentationProjection` now tokenizes RAW into editor HTML, preview HTML,
  logical text, and logical line metadata for that snapshot, while `ContentsEditorTypingController.qml` carries the
  incremental plain-text/source-offset state between idle commits.
- `ContentsInlineFormatEditor.qml` is now strictly a plain-text input wrapper.
  The fallback editor path paints formatted spans through a separate HTML overlay and no longer switches the live
  `TextEdit` into `RichText` mode.
- Desktop/mobile focused editing now also blocks that whole-document presentation timer from pushing a fresh editing
  surface back into the live `TextEdit`.
  Ordinary typing stays on the incremental live cache until blur or another explicit immediate refresh path, which
  prevents dropped keys, Hangul jamo deletion, and stale cursor restoration during active note writing.
- `ContentsEditorTypingController.qml` now also maintains incremental logical line-start offsets and pushes the entire
  live state into `ContentsLogicalTextBridge.adoptIncrementalState(...)`, so bridge consumers stay current without a
  whole-note rebuild per keystroke.
- Desktop/mobile minimap snapshotting now also shares `ContentsMinimapSnapshotSupport.js`, so ordinary note edits only
  re-sample the changed logical-line window instead of walking the full note text through `positionToRectangle(...)`.
- Desktop/mobile line geometry helpers now reuse that same logical-line group cache even when the minimap is hidden, so
  gutter line-Y queries no longer need their own whole-document geometry sweep.
- `ContentsInlineFormatEditor.qml` now also owns the `Qt.inputMethod.update(...)` bridge for cursor, selection, and
  geometry changes, keeping mobile platform text-selection handles and iOS keyboard trackpad gestures aligned with the
  live `TextEdit`.
- Both the wrapper and the selection controller now restore selections with `TextEdit.moveCursorSelection(...)` when an
  active edge matters, so OS-driven selection expansion keeps one continuous anchor instead of degrading to repeated
  fresh sub-selections.
- `ContentsInlineFormatEditor.qml` now also supplements mobile native-input selection with passive touch multi-tap
  handling, restoring double-tap word selection and triple-tap paragraph selection even though the live editor sits
  inside a `Flickable`.
- In mobile native-input mode, the shared wrapper now also separates scroll-vs-edit activation:
  - keyboard hidden: touch `press`/drag stays scroll-first and does not immediately move cursor/focus
  - single tap: places cursor at the tapped point and opens the keyboard
  - keyboard visible: normal cursor placement/selection remains enabled
- Mobile editor hosts now opt into native-input priority rules, pause note snapshot polling, delay app-driven
  RichText surface reinjection until the OS input session settles, and use a plain logical-text input surface instead
  of the RichText editor projection.
  Native-input priority no longer carries a deferred-persistence exception; mobile shares the same immediate
  `.wsnbody` flush contract as desktop.
- Desktop editor hosts now also pause note snapshot polling while the live editor owns focus, so the periodic
  selected-note snapshot refresh cannot overwrite the active buffer with a stale same-note payload mid-typing.
- The unified `ContentsDisplayView.qml` now also removes the live-editor horizontal inset in mobile mode, so the
  content view spans the full routed mobile width without a separate mobile host file.
- The unified host now also keeps a dedicated gutter-Y cache that accumulates prior soft-wrap row height before placing
  later logical line numbers, so wrapped prose no longer leaves gutter labels visually under-shifted.
- `ContentsStructuredDocumentFlow.qml` now also emits structured gutter Y from each cached logical line's real
  `contentY` instead of a separate synthetic gutter accumulator, so paragraph spacing, delegate-local top offsets, and
  wrapped structured lines keep the gutter aligned to the actual rendered document surface.
- `ContentsEditorSelectionController.qml` now also owns common markdown list shortcuts (`Cmd+Shift+7/8` on macOS,
  `Alt+Shift+7/8` on Windows/Linux) so block-level list toggles stay source-driven as well.
- Markdown list toggles now restore selection/cursor using logical-text lengths from `ContentsLogicalTextBridge`, so
  lines that contain inline tags, escaped entities, or `<resource ...>` tokens no longer jump selection after a list
  rewrite.
- `ContentsEditorSession.qml` no longer serializes note swaps behind pending-body save barriers. It keeps the live editor buffer
  authoritative, re-stages the old note into the buffered fetch-sync controller, and allows the next selected note to
  bind immediately.
- `ContentsEditorSession.qml` now also keeps local editor authority across same-note model echoes, so one successful
  echo cannot immediately reopen the door for the next stale polling snapshot to replace the current note body.
- `ContentsEditorSession.qml` now also gates same-note model snapshot apply with an explicit typing-idle window
  (`typingIdleThresholdMs` + last local edit timestamp), so non-idle typing turns cannot be overwritten by an older
  snapshot captured before the latest user input.
- Desktop/mobile hosts now inject that idle threshold through `editorIdleSyncThresholdMs`, keeping the apply policy
  consistent across both surfaces.
- Desktop/mobile editor views now also gate timer-driven snapshot polling and deferred presentation commits on
  `typingSessionSyncProtected` plus `pendingBodySave`, not only focus state, so stale async snapshots cannot overwrite
  active typing when focus reporting briefly flaps.
- `ContentsDisplayView.qml` now delegates three former host-owned policy roles to C++ coordinators under
  `src/app/viewmodel/content`: note-selection sync/reconcile scheduling, whole-document presentation refresh policy,
  and structured-flow activation. The unified host keeps UI composition and repaint/focus execution only, while
  `ContentsDisplayHostModePolicy.qml` carries platform-mode presentation policy.
- `ContentsStructuredDocumentFlow.qml` now also converges structured document-host state through one
  `ContentsStructuredDocumentHost` instance and delegates collection normalization, focus resolution, and RAW mutation
  rules to dedicated C++ policy objects instead of keeping those host policies interleaved inside the QML flow object.
- After that policy extraction, the flow and desktop/mobile hosts no longer keep dead duplicate QML helpers or
  pass-through import wrappers that merely mirrored those dedicated collaborators.
- Desktop/mobile snapshot polling now also prefers a filesystem reconcile fetch path
  (`reconcileViewSessionAndRefreshSnapshotForNote(noteId, editorSession.editorText)`) instead of only running
  model-side snapshot reload ticks.
  This shifts sync balance toward RAW fetch verification while still keeping write staging eventual.
- Desktop/mobile hosts now also bind `ContentsEditorSelectionController` and
  `ContentsEditorTypingController` to `contentsView.contentEditor` explicitly.
  The previous self-referential `contentEditor: contentEditor` assignment created runtime binding loops under
  `pragma ComponentBehavior: Bound`, which could leave selection/typing/save orchestration detached from the real
  editor surface.
- That reconcile path is now request/complete based rather than synchronous:
  - desktop/mobile hosts queue one note-entry reconcile per selected note
  - `ContentsEditorSelectionBridge` exposes completion through `viewSessionSnapshotReconciled(...)`
  - note-open and timer-driven reconciliation no longer perform RAW note reads on the UI thread
  - timer-driven polling now also respects one in-flight reconcile per selected note instead of enqueueing overlapping
    duplicate fetches
- Selected note bodies are now also lazy-loaded:
  - library/bookmarks/projects/progress note-list rows carry preview/search metadata, not the full note body
  - `ContentsEditorSelectionBridge` exposes `selectedNoteBodyLoading` while the selected note body is read on a worker
    thread
  - desktop/mobile hosts defer `requestSyncEditorTextFromSelection(...)` until that body read completes
  - desktop/mobile hosts also schedule one more selection-sync pass when that loading flag returns to `false`, so
    empty-body notes still clear the previous session even when `selectedNoteBodyText` remains `""`
  - a large note-open therefore no longer requires the note-list model, the selection bridge, and the editor session to
    all duplicate the same full body text at once
- Desktop/mobile note transitions now also project any still-live `TextEdit` delta through
  `ContentsEditorTypingController.handleEditorTextEdited()` before flushing the previously bound note.
  Combined with the bridge-side pending-body adoption path, this keeps large deletions from being dropped or replaced
  by a stale package read when the user briefly visits another note and comes back.
- That selection-change preflush now runs only while the editor still owns focus, and unchanged direct body persists
  now short-circuit in `WhatSonLocalNoteFileStore`.
  Selecting a note without editing it therefore must not touch `lastModifiedAt`, must not look like a save, and must
  not move the note to the top of the list.
- Agenda/callout/break/resource notes enter the structured document-flow host.
  Desktop/mobile hosts therefore keep image/resource drops on the parser-owned block renderer instead of reintroducing
  a dedicated resource surface or body-overlay fallback.
- That structured document-flow host now activates only after the parser has produced actual rendered document blocks.
  A raw `<resource ... />` token alone therefore cannot prematurely displace surrounding prose with an image-first
  fallback surface.
- `ContentsEditorTypingController.qml` now also refuses to run its legacy whole-editor diff pass while
  `ContentsStructuredDocumentFlow.qml` is active.
  Structured notes therefore no longer risk `<resource ... />` damage when note-switch cleanup tries to flush the old
  editor surface.
- That same legacy diff path now also requires a real mounted inline editor surface.
  When the host has already fallen back to the proxy object during note switching, the controller no longer treats that
  proxy as editable input and no longer rewrites the old note body from a non-existent editor snapshot.
- `ContentsInlineFormatEditor.qml` now emits committed typing directly from the nested `TextEdit.onTextChanged` path
  whenever the change is not programmatic and IME composition has already settled.
  That keeps `ContentsEditorSession.editorText` moving with the visible buffer instead of leaving the note-open body
  snapshot as the last authoritative session text.
- `ContentsEditorTypingController.qml` no longer reverse-normalizes the whole rendered RichText surface back into RAW
  during ordinary typing.
  Agenda/callout rendering is intentionally lossy at the surface layer, so rebuilding source from that HTML could let
  a stale note-open session snapshot overwrite later visible edits.
- `ContentsEditorTypingController.qml` now rebuilds post-edit logical line-start offsets from the resulting logical
  text each mutation, fixing stale line-count growth where gutter/minimap lines could increase but not decrease after
  newline removal or line-wrap collapse.
- Editor body persistence is now split into immediate-flush requests plus buffered retry/completion:
  - `ContentsDisplayView.qml` now mounts `ContentsEditorSessionController` directly on the main editor path
  - `ContentsEditorSession.qml` remains only as a compatibility wrapper
  - `ContentsEditorSessionController` owns pending/in-flight state, same-note RAW sync acceptance, and text
    normalization for persistence/model-sync turns
  - `ContentsEditorSelectionBridge` stays as the QML-facing adapter
  - `file/sync/ContentsEditorIdleSyncController` owns the note-scoped buffered snapshot cache and recurring `1000ms`
    fetch clock
  - `ContentsNoteManagementCoordinator` serializes direct `.wsnote` writes plus open-count/stat follow-up work on the
    downstream management side
  - selection/typing controllers now default to immediate `.wsnbody` flush requests for live editor mutations, while
    the buffered fetch clock remains the retry/drain path for dirty note snapshots that were already accepted into the
    persistence controller
  - immediate flush calls now return `false` when the persistence lane rejects the current snapshot, so note-switch
    flows no longer treat a rejected enqueue as if the note had already been durably accepted
  - each successful queued write now also triggers one reconcile verify(fetch) against filesystem RAW, so the visible
    note snapshot converges even when downstream body serialization canonicalizes markup/escaping.
  - same-note RAW/model snapshot rejection while the editor still has local-protection state no longer schedules a new
    persistence write from that rejection alone; the editor now waits for an explicit local mutation path instead of
    using model-sync refusal as a reverse write trigger
  - the save-path policy no longer depends on QML `undefined` checks or JavaScript regex normalization; those rules now
    execute in the typed C++ session controller
  - on Android SAF-mounted hubs, the successful note-management path now mirrors the locally written note package back
    into the original source document tree before the editor receives a final save success signal
  - when desktop/mobile hosts call `persistEditorTextImmediately(...)`, that path now issues one immediate fetch enqueue
    attempt for the current note payload instead of silently downgrading to deferred-only staging
- The RichText editor surface now decodes one safe-entity layer for display, so RAW-preserving source escapes like
  `&lt;` / `&gt;` / `&amp;` render as visible glyphs without changing the canonical note-body source contract.
- The print-page `Repeater` delegates now declare `required property int index`, and the inline editor host now uses a
  concrete `shapeStyle: 0` value instead of an undefined `shapeRoundRect` symbol, removing two runtime
  `ReferenceError` classes observed during live app execution.
- Gutter/minimap/editor default geometry now routes through LVRS `gap`, `stroke`, theme-color, and `scaleMetric(...)`
  tokens instead of scattered local editor pixel literals.
- The desktop gutter layout is now hard-clamped to its resolved token width, so markdown-list relayouts cannot make the
  gutter visibly squeeze or rebound while typing.
- Markdown list continuation on `Enter` is also owned by `ContentsEditorTypingController.qml`, so bullet/numbered list
  continuation stays source-driven instead of relying on RichText widget heuristics or the editor's rendered bullet
  glyph representation.
- `ContentsEditorTypingController.qml` now also canonicalizes a standalone `---` typing line into the proprietary
  divider source token `</break>` before persistence.
- `ContentsEditorTypingController.qml` now also owns divider authoring shortcuts:
  - `Cmd+Shift+H` inserts canonical `</break>` into RAW at the current cursor
  - `Ctrl+Shift+H` fallback is also accepted when runtime Command mapping resolves as `ControlModifier`
- `ContentsEditorTypingController.qml` now also owns agenda authoring shortcuts:
  - `Cmd+Opt+T` inserts canonical `<agenda date="YYYY-MM-DD"><task done="false"> </task></agenda>` (empty-body
    cursor anchor included)
  - `Ctrl+Alt+T` fallback is also accepted when runtime Command mapping resolves as `ControlModifier`
  - insertion now aborts unless that payload still validates as one complete `<agenda><task>...</task></agenda>` block
  - markdown-like `[] item` / `[x] item` lines are rewritten into agenda/task source blocks
  - pressing `Enter` inside `<task>` either creates the next `<task>` or exits agenda editing when the current task is
    empty
  - if agenda exit occurs on an empty task and all sibling tasks are empty, the entire agenda block is removed
- `ContentsEditorTypingController.qml` now also owns callout authoring shortcuts:
  - `Cmd+Opt+C` inserts canonical `<callout> </callout>` (empty-body cursor anchor included) into RAW at the current cursor
  - `Ctrl+Alt+C` fallback is also accepted when runtime Command mapping resolves as `ControlModifier`
  - insertion now aborts unless that payload still validates as one complete `<callout>...</callout>` block
  - pressing `Enter` twice on a trailing empty callout line exits the callout block on the second `Enter`
- `ContentsStructuredCursorSupport.js` now centralizes plain-text cursor/source-offset mapping for agenda/callout block
  reparses, so local caret restoration survives entity-escaped RAW rewrites.
- `ContentsStructuredCursorSupport.js` now also carries inline-tag-aware text-block cursor/source mapping, letting
  structured text blocks edit RAW source spans directly from plain-text deltas instead of round-tripping rendered HTML
  through a DOM-to-source normalization step.
- `ContentsEditorDebugTrace.js` now centralizes verbose editor-domain QML tracing, and the desktop/mobile display
  hosts, session bridge, typing controller, structured document flow, block wrapper, and inline editor now emit
  lifecycle, mount/unmount, selection-sync, source-mutation, cursor/selection, and focus-transition logs through that
  shared helper instead of each file inventing a different console format.
- `ContentsStructuredDocumentFlow.qml` now asks the active block delegate for structured shortcut insertion offsets,
  which restores live-caret insertion for text blocks while still keeping agenda/callout shortcuts block-scoped.
- If the structured host temporarily has no interactive block, that same insertion path no longer falls back to the
  last block end; callers now use a pending cursor source offset or fall back to the outer cursor bridge.
- That same structured-flow insertion bridge now also accepts dropped resource-tag batches, so resource imports inside
  agenda/callout/break notes insert next to the active block instead of appending through the legacy cursor bridge.
- `ContentsStructuredBlockRenderer.*` now also short-circuits notes that contain no proprietary structured tags and
  computes combined structured verification only once per source refresh, shrinking note-open parse overhead.
- In structured-flow mode, desktop/mobile hosts now stop feeding legacy agenda/callout overlay layers, so the fallback
  rendering path does not instantiate off-screen delegates alongside the document-native flow.
- `ContentsStructuredDocumentFlow.qml` now also loads larger block lists asynchronously and replays pending focus once a
  delegate becomes available, reducing synchronous note-open stalls.
- Structured-flow focus restoration now resolves one target block index per request and calls that delegate directly,
  replacing the earlier whole-tree focus fan-out across every block, loader, and nested editor.
- That focus path no longer keeps an incrementing replay token or a generic `pendingFocusRequestChanged` watcher; it now
  recalculates the target block only when a focus request enters or the reparsed block list actually changes.
- Structured-flow source edits no longer force `ContentsDisplayView.qml` to rebuild the whole legacy presentation
  snapshot on each keystroke; the unified host now only repopulates the fallback RichText surface when the document
  leaves structured mode again.
- While structured-flow mode is active, both hosts now unload the legacy `ContentsInlineFormatEditor` through a
  `Loader`, so note-open and block-edit turns no longer keep a second full-document editor instance alive behind
  `visible: false`.
- Those hosts keep a lightweight proxy object under the shared `contentEditor` reference, preserving existing
  geometry/focus helper call sites without requiring the hidden editor instance to stay mounted.
- Desktop/mobile hosts now also treat `editorSession.editorTextSynchronized` as the main post-sync refresh boundary,
  which removes duplicate minimap/presentation/gutter refresh scheduling after model sync, reconcile completion, and
  structured correction apply.
- Desktop/mobile hosts now also merge note-open selection work through one queued selection-sync helper per event-loop
  turn, so `selectedNoteId` and `selectedNoteBodyText` updates for the same selected note no longer replay the same
  session-sync and fallback-refresh work twice.
- Desktop/mobile hosts now also route visibility re-entry through that same queued selection-sync helper, so reopening
  the editor surface does not schedule a second parallel note-open refresh path next to the selected-note handlers.
- Desktop/mobile hosts now also bind `ContentsStructuredBlockRenderer.backgroundRefreshEnabled` to the note-open/model
  sync window, not only to the unfocused state.
- Newly selected notes stay on the legacy editor/session path until the first settled structured render confirms block
  ownership, then later same-note async reparses keep the structured surface mounted while agenda/callout parsing runs
  off the UI thread.
- Structured shortcut insertion now also resolves out of existing proprietary wrappers before writing:
  - invoking agenda/callout insertion while the cursor is already inside an existing agenda/callout moves the new RAW
    block to the enclosing wrapper end first
  - newline padding is then applied around that resolved point so proprietary blocks remain standalone instead of
    nesting inside one another
- Agenda Enter handling now distinguishes empty trailing tasks from empty middle tasks:
  - trailing empty task exits the agenda as before
  - empty middle task removes only that task instead of deleting later sibling tasks
- Agenda empty-body detection now decodes visible text consistently, so entity-only task bodies such as `&amp;` do not
  get treated as blank and removed by the empty-task exit path.
- The plain agenda overlay checkbox path now ignores renderer-echo `checked` changes, preventing redundant `done`
  rewrites when renderer models refresh after a toggle.
- Callout block focus restoration now also refreshes the flow host's active-block pointer on cursor-only programmatic
  focus moves, keeping subsequent structured shortcuts scoped to the active callout.
- Agenda parsing and source-mutation backend logic used by those shortcuts now lives in
  `src/app/agenda/ContentsAgendaBackend.*`, while QML controllers keep event/cursor orchestration only.
- Callout parsing and insertion payload backend logic now lives in
  `src/app/callout/ContentsCalloutBackend.*`, while QML controllers keep event/cursor orchestration only.
- Agenda/callout render-model projection now lives in
  `src/app/editor/renderer/ContentsStructuredBlockRenderer.*`, so QML overlay layers consume renderer-owned data
  instead of calling parse backends directly.
- `ContentsEditorSession.qml` now treats `date="yyyy-mm-dd"` as a modification-time placeholder:
  - when local note modification is staged for persistence, placeholder dates are rewritten to current `YYYY-MM-DD`
  - passive same-note model sync does not rewrite agenda dates
- `ContentsEditorSession.qml` now also normalizes empty structured blocks into one-space anchors on model sync:
  - `<task ...></task>` -> `<task ...> </task>`
  - `<callout></callout>` -> `<callout> </callout>`
- `ContentsAgendaLayer.qml` is now mounted by desktop/mobile hosts for every editor view mode, including `Plain`, and
  renders agenda cards with `LV.CheckBox` task rows bound to source `done` attributes via renderer-provided agenda
  models.
- `ContentsCalloutLayer.qml` is now mounted by desktop/mobile hosts for every editor view mode, including `Plain`,
  and renders Figma-aligned callout rows from renderer-provided canonical `<callout>...</callout>` models.
- Agenda/callout layers now consume parser-returned `sourceStart` offsets and host `sourceOffsetYResolver(...)`
  callbacks, so structured cards are placed at authored source-tag positions in the editor viewport.
- Those render models now also expose `focusSourceOffset`, and desktop/mobile hosts route card taps back into the
  live editor cursor path so empty agenda/callout cards remain editable as soon as the underlying RAW tag exists.
- Agenda/callout placement now resolves against editor-content-relative document Y only, preventing double-counted top
  offsets from pushing cards out of view after tag insertion.
- Empty continued markdown list items now also break back to a plain blank line on `Enter`, so repeated list newlines do
  not get stuck in an endless empty-list state.
- `Page` / `Print` now mount the live RichText editor inside an outer paper-document viewport, so the paper grows with
  the note instead of remaining a fixed-height scaffold.
- `Page` / `Print` paper visuals now use an A4-style off-white sheet gradient with per-page separators and subtle
  shadow depth, replacing the prior plain-white flat backdrop.
- `Page` / `Print` mode gating plus paper geometry/page-count math are now provided by
  `ContentsPagePrintLayoutRenderer` in `src/app/editor/renderer`, so editor QML hosts consume backend layout state.
- The repository no longer operates scripted editor tests; the per-file regression notes in this directory are
  documentation-only behavior contracts.
- `docs/STRUCTURED_EDITOR_REGRESSION_CHECKLIST.md` now captures the manual regression cases for structured-flow caret
  persistence and live-caret shortcut insertion.
- Cursor restoration for ordinary typing/focus recovery now routes through the wrapper-level cursor setter instead of
  rewriting `cursorPosition` into the wrapper, `editorItem`, and `inputItem` together.
- The shared editor wrapper no longer depends on its own synthetic IME commit queue for ordinary typing, so a just-typed
  word is no longer left behind a wrapper-local composition flag while the user immediately clicks to move the cursor.
- The shared editor wrapper now also prefers the native `QtQuick.TextEdit` edited-signal / input-method commit path
  instead of maintaining its own synthetic IME commit flag, aligning Hangul composition behavior with standard text
  editors and word processors more closely.
- That same wrapper now also suppresses one echoed native `textEdited` turn for host-driven RichText surface
  replacement, preventing debounced presentation refresh from re-entering the RAW typing-mutation path as a fake user
  edit.
- The shared editor wrapper now also normalizes tab indentation width through `TextEdit.tabStopDistance` using runtime
  font metrics for four spaces, so Tab indent depth no longer jumps to an oversized default column.
- The shared editor wrapper now also intercepts direct `Tab` key insertion to emit four literal spaces by default,
  preventing mode-dependent `\t` expansion from producing oversized indentation width.
- `ContentsEditorTypingController.qml` no longer drops a committed `textEdited` mutation only because a transient
  model-sync guard bit is still set; committed user typing now always refreshes local authority and persistence staging.
- The shared selection controller now also primes right-click context-menu selections on mouse press, so multi-block or
  mixed-inline dragged selections survive the menu-opening click instead of collapsing to one fragment before
  formatting.
- Shared inline-format actions now also rebuild proprietary source tags from RAW logical coverage, so formatting ranges
  that cross multiple paragraphs or existing inline tags stay governed by canonical `<bold>` / `<italic>` / ...
  boundaries instead of temporary RichText fragment splits.
- Agenda source tags must now round-trip through persistence without escaping (`<agenda>`, `<task>`) and keep canonical
  attribute forms (`date=YYYY-MM-DD`, `done=true|false`).
- Callout source tags must now round-trip through persistence without escaping (`<callout>...</callout>`).
- Agenda/callout/divider RAW blocks must now also round-trip as standalone body children on disk instead of being
  rewrapped into `<paragraph>`.

## Intended Detailed Sections
- Module responsibilities and architectural layer
- Internal submodule boundaries
- Cross-directory dependencies
- Runtime ownership and lifecycle rules
- Testing strategy and coverage map
- Known hotspots and refactor priorities
