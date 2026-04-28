# `src/app/qml/view/content/editor`

## Scope
- Mirrored source directory: `src/app/qml/view/content/editor`
- Child directories: 0
- Child files: 25

## Child Directories
- No child directories.

## Child Files
- `ContentsAgendaLayer.qml`
- `ContentsAgendaBlock.qml`
- `ContentsBreakBlock.qml`
- `ContentsCalloutLayer.qml`
- `ContentsCalloutBlock.qml`
- `ContentsDisplayAuxiliaryRailHost.qml`
- `ContentsDisplayExceptionOverlay.qml`
- `ContentsDisplayGutterHost.qml`
- `ContentsDisplayMinimapRailHost.qml`
- `ContentsDisplayOverlayHost.qml`
- `ContentsDisplayResourceImportConflictAlert.qml`
- `ContentsDisplaySurfaceHost.qml`
- `ContentsDisplayView.qml`
- `ContentsDocumentBlock.qml`
- `ContentsDocumentTextBlock.qml`
- `ContentsGutterLayer.qml`
- `ContentsImageResourceFrame.qml`
- `ContentsInlineFormatEditor.qml`
- `ContentsMinimapLayer.qml`
- `ContentsResourceBlock.qml`
- `ContentsResourceViewer.qml`
- `ContentsResourceRenderCard.qml`
- `ContentsResourceEditorView.qml`
- `ContentsResourceLayer.qml`
- `ContentsStructuredDocumentFlow.qml`

## Current Notes

- `ContentsDisplayView.qml` is now the unified desktop/mobile editor surface.
- `ContentsDisplayView.qml` delegates view-only surface, auxiliary rail, and overlay composition to narrow sibling
  hosts: `ContentsDisplaySurfaceHost.qml`, `ContentsDisplayAuxiliaryRailHost.qml`, and
  `ContentsDisplayOverlayHost.qml`.
- `ContentsDisplayView.qml` restores note-selection focus directly through
  `ContentsStructuredDocumentFlow.qml`, keeping the parser-backed structured host as the only note-editor focus target.
- `ContentsDisplaySurfacePolicy` now owns the note-surface decision: selected notes mount the structured document flow,
  and the unreachable whole-note inline loader is no longer part of `ContentsDisplayView.qml`.
- The editor surface no longer mounts a note-loading overlay. Mount decisions now settle through
  `ContentsDisplayNoteBodyMountCoordinator::mountDecisionClean`, so a stale pending body load cannot dim already
  rendered note content.
- Focus/input availability is likewise tied to parse-mounted note-body state, so a parse-mounted note stays editable
  while slower startup runtime state finishes settling.
- Tag-management command surfaces derive directly from that same parse-mounted RAW state plus active structured-editor
  mode, so inline formatting shortcuts can insert RAW `<bold>`/`<italic>`/`<highlight>` tags before the slower
  editor-session `noteMounted` flag completes and without a second command-surface-ready proxy.
- `ContentsDisplayView.qml` no longer mirrors extra `surfaceReady`, `surfaceInteractive`, or
  `noteDocumentCommandSurfaceEnabled` aliases on top of that RAW state.
  The parser-backed `.wsnbody` mount is the only editor-readiness authority exposed to the input/view layer.
- This directory now contains only editor view hosts, visual layers, and block delegates. Non-visual QML policies,
  controllers, session controllers, and support JavaScript live under `src/app/models/editor`.
- `src/app/models/editor/display/ContentsDisplayHostModePolicy.qml` owns platform display deltas such as
  gutter/minimap visibility, editor horizontal inset, and font weight so the shared host no longer forks into separate
  desktop/mobile QML roots.
- `ContentsDisplayView.qml` now also keeps gutter-body spacing separate from editor text padding through a dedicated
  `gutterBodyGap` token, so line numbers can sit closer to the note body without shrinking the body column itself.
- `ContentsDisplayView.qml` now scales the existing non-print bottom accessibility inset up to roughly half of the
  live editor height, so end-of-document editing keeps a much larger visual landing area without introducing a second
  spacer concept.
- The shared live editor engine is already `QtQuick.TextEdit` wrapped by `ContentsInlineFormatEditor.qml`; this
  directory no longer depends on an LVRS `LV.TextEditor` implementation.
- The editor, gutter, and minimap fill the `ContentsView` slot; note-body resources no longer use a separate overlay
  host above the editor.
- `ContentsResourceRenderCard.qml` now centralizes the shared desktop/mobile inline resource card used inside parsed
  note-body blocks, so file/image/audio/pdf/document presentation no longer duplicates the same card scaffolding in
  both hosts.
- `src/app/models/editor/resource/ContentsResourceImportController.qml` is now only the public coordinator for
  editor-side resource import.
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
- `ContentViewLayout.qml` now selects one sibling surface policy for this directory:
  - `ContentsDisplayView.qml` remains the note editor host for note-backed hierarchies
  - `ContentsResourceEditorView.qml` becomes the dedicated center-surface resource editor when the active
    note-list model exposes a direct resource selection instead of a note-backed document session
- That dedicated resource editor surface is now intentionally transparent and viewer-only, so Resources hierarchy
  browsing does not add a second metadata card or explanatory copy above/below the actual asset preview.
- `src/app/models/editor/display/ContentsEditorSurfaceModeSupport.js` owns that QML-side center-surface decision so
  `ContentViewLayout.qml`
  does not duplicate the note-backed/resource-backed detection logic inline.
- Resource-bearing note bodies now activate `ContentsStructuredDocumentFlow.qml` so `<resource ... />` stays in the
  same authored block stream as surrounding text.
- `ContentsStructuredDocumentFlow.qml` now mounts one generic `ContentsDocumentBlock.qml` delegate per parsed block
  instead of branching directly to block-type-specific delegates in the repeater host.
- That structured flow now also feeds its repeater through `ContentsStructuredDocumentBlocksModel` rather than
  binding parsed block arrays directly.
  Single-line delete and similar RAW edits therefore preserve unchanged block delegates instead of remounting the
  entire structured editor column when only source offsets moved.
- Paragraph/p prose blocks keep a shared RAW mutation-policy helper for non-native hosts, but the shared inline editor
  no longer routes ordinary `Enter`, `Backspace`, or `Delete` through QML key interception.
  Live note-body text editing now leaves those keys with the OS/Qt `TextEdit` path.
- Parser payloads and mounted block delegates now share one block contract:
  `plainText`, `textEditable`, `atomicBlock`, `gutterCollapsed`, `logicalLineCountHint`,
  `minimapVisualKind`, and `minimapRepresentativeCharCount`.
  Gutter, minimap, current-line focus, and nearest-editable-block resolution now consume that shared contract rather
  than branching on block type names inside the flow host.
- Structured minimap rows now follow the parser-normalized block stream directly.
  One complete block/tag becomes one minimap row; text-like rows size themselves from the amount of block plain text,
  while resource rows stay block-like.
- `ContentsDocumentTextBlock.qml` uses a read-side HTML overlay only when RAW block source contains inline style tags.
  Structured paragraph editing still happens directly against RAW block source text, while the overlay renders tags such
  as `<bold>`, `<italic>`, and `<highlight>` as visible formatting.
- `ContentsResourceBlock.qml` no longer keeps a resource-local `before/selected/after` boundary-editor state machine.
  Resource rows now behave as plain atomic document blocks that emit selection, deletion, and boundary-navigation
  requests back to the flow host.
- Block-boundary keyboard navigation is now being consolidated under `ContentsStructuredDocumentFlow.qml` itself.
  Text/resource/break/callout/agenda delegates are expected to emit generic boundary-navigation requests and let the
  flow resolve the immediately adjacent parsed block from the `.wsnbody` stream rather than hardcoding neighbor lookup
  rules inside each block widget.
- `src/app/models/editor/input/ContentsDocumentTextBlockController.qml` now owns ordinary structured text-block
  mutation routing. Plain text blocks commit the live `TextEdit` plain text directly as the next RAW block source,
  while styled blocks keep the inline-tag-aware replacement path so formatting tags survive visible text edits.
- The editor directory now follows one write direction for live note editing:
  RAW `.wsnote/.wsnbody` source is the only write authority, parsers/builders derive presentation state from that RAW
  source, and HTML/DOM presentation surfaces are no longer allowed to serialize themselves back into stored source
  during ordinary typing.
- The live input-to-render path is intentionally short: native `TextEdit` input or tag-management commands build the
  next RAW source, the display mutation controller writes that RAW source directly to `editorText`, and the existing
  parser/renderer projections observe that source change. Display-mode mutation plans must not sit between input and
  the RAW write.
- Desktop/mobile editor views now keep a separate presentation timer for whole-document markdown/HTML projection refresh, so
  `ContentsTextFormatRenderer` and full minimap resampling no longer run directly on every committed keystroke.
- Desktop/mobile editor views now also keep `documentPresentationSourceText` as the single whole-document presentation
  snapshot. One `ContentsEditorPresentationProjection` now tokenizes RAW into editor HTML, preview HTML,
  logical text, and logical line metadata for that snapshot, while `ContentsEditorTypingController.qml` carries the
  incremental plain-text/source-offset state between idle commits.
- `ContentsInlineFormatEditor.qml` is now strictly a plain-text input controller.
  The fallback editor path paints formatted spans through a separate HTML overlay and no longer switches the live
  `TextEdit` into `RichText` mode.
- Desktop/mobile focused editing now also blocks that whole-document presentation timer from pushing a fresh editing
  surface back into the live `TextEdit`.
  Ordinary typing stays on the incremental live cache until blur or another explicit immediate refresh path, which
  prevents dropped keys, Hangul jamo deletion, and stale cursor restoration during active note writing.
- Blur no longer forces immediate persistence after a fixed retry count while OS IME composition/preedit is still
  active; the unsettled native input session is left untouched instead of being pushed into RAW save.
- `ContentsEditorTypingController.qml` now also maintains incremental logical line-start offsets and pushes the entire
  live state into `ContentsLogicalTextBridge.adoptIncrementalState(...)`, so bridge consumers stay current without a
  whole-note rebuild per keystroke.
- Desktop/mobile minimap snapshotting now also shares `ContentsMinimapSnapshotSupport.js`, so ordinary note edits only
  rebuild the changed snapshot slice instead of rebuilding the whole rail.
  On structured notes the cached rows now represent parser-backed block silhouettes rather than live per-line text
  rectangles, keeping the minimap aligned with `.wsnbody` block order.
- Desktop/mobile line geometry helpers now reuse that same logical-line group cache even when the minimap is hidden, so
  gutter line-Y queries no longer need their own whole-document geometry sweep.
- Desktop/mobile editor hosts now also mirror `ContentsEditorPresentationProjection` logical-line metrics into
  explicit host state and listen to `logicalLineCountChanged` plus `logicalLineStartOffsetsChanged` directly, so
  gutter line numbers refresh on the same RAW-to-projection turn instead of waiting for incidental UI movement.
- Real gutter geometry invalidations from structured-flow re-layout and inline resource render changes now bypass the
  focused `line-structure` suppression path and request their own gutter refresh reasons.
- The unified host now also fingerprints structured gutter geometry separately from logical line-count metadata.
  A resource-driven spacing change therefore refreshes gutter Y even when the parsed logical line count did not
  change at all.
- `ContentsInlineFormatEditor.qml` no longer owns a QInputMethod notification bridge. Cursor, selection, preedit text,
  candidate placement, keyboard visibility, and query updates stay on the live OS/Qt `TextEdit` path.
  The editor must not call `Qt.inputMethod.*`, the bare QML `InputMethod.*` singleton, or maintain fallback branches
  for alternate input-method objects; the C++ regression suite scans the QML source tree for those forbidden patterns.
- Both the controller and the selection controller now restore selections with `TextEdit.moveCursorSelection(...)` when an
  active edge matters, so OS-driven selection expansion keeps one continuous anchor instead of degrading to repeated
  fresh sub-selections.
- `ContentsInlineFormatEditor.qml` now leaves mobile and desktop pointer selection directly on the live `TextEdit`.
  It no longer mounts a transparent guard layer or touch multi-tap handler above the input surface, so OS/Qt cursor
  placement, selection handles, word selection, and drag selection are not preempted by controller code.
- The shared live `TextEdit` controller now also declares every available Qt keyboard/selection feature flag instead of
  relying on implicit defaults: focus-on-press, keyboard selection, pointer selection, persistent selection,
  unrestricted input-method hints, character-level mouse selection, and insert-mode editing stay enabled for all note
  body editors.
- Editor hosts now opt into native-input priority rules on every platform, pause note snapshot polling, delay app-driven
  RichText surface reinjection until the OS input session settles, and use a plain logical-text input surface instead
  of the RichText editor projection.
  Native-input priority no longer carries a deferred-persistence exception; every host shares the same immediate
  `.wsnbody` flush contract as desktop.
- Native-input priority now also disables structured text-boundary key interception and window-level document
  shortcuts while the OS keyboard owns the session.
  Continuous Backspace, selection gestures, and IME candidate/control gestures therefore stay on the platform `TextEdit`
  path instead of being accepted by QML handlers.
- Inline editor, text block, callout, agenda, break, and document-block delegates now keep their non-visual input state
  in `src/app/models/editor/input/*Controller.qml` objects. View QML in this directory should compose LVRS/Qt visual
  surfaces, expose signals/properties, and delegate live typing, cursor bookkeeping, source replacement, selection
  cache, and atomic tag-management decisions to those model-side controllers.
- `ContentsEditorInputPolicyAdapter.qml` now centralizes native-input session policy for the editor domain. The host,
  structured text blocks, and `ContentsInlineFormatEditor.qml` route shortcut gating, long-press gating, focused
  programmatic sync, and ordinary text-edit focus-restore decisions through that adapter instead of each QML file
  reimplementing its own IME or gesture condition.
- Editor custom input is now policy-locked off by default through `editorCustomTextInputEnabled: false`.
  Only tag-management commands may sit outside the native `TextEdit` input path: inline style tags, resource paste,
  agenda/callout/break source-tag insertion, and selected atomic resource/break block management.
- Clipboard-image paste is routed through the live `TextEdit` tag-management hook only after the key event matches the
  platform paste shortcut and the resource import controller confirms an importable image. If the hook declines, the
  event is released back to native text paste.
- Markdown list shortcuts and markdown list Enter continuation are intentionally absent from the editor input layer.
  Users can still type markdown marker text literally, but the editor does not intercept list keys or synthesize the
  next list line.
- Desktop editor hosts now also pause note snapshot polling while the live editor owns focus, so the periodic
  selected-note snapshot refresh cannot overwrite the active buffer with a stale same-note payload mid-typing.
- The unified `ContentsDisplayView.qml` now also removes the live-editor horizontal inset in mobile mode, so the
  content view spans the full routed mobile width without a separate mobile host file.
- The unified host now also keeps a dedicated gutter-Y cache that accumulates prior soft-wrap row height before placing
  later logical line numbers, so wrapped prose no longer leaves gutter labels visually under-shifted.
- `ContentsStructuredDocumentFlow.qml` now also emits structured gutter Y from each cached logical line's real
  `contentY` instead of a separate synthetic gutter accumulator, so paragraph spacing, delegate-local top offsets, and
  wrapped structured lines keep the gutter aligned to the actual rendered document surface.
- Text-family structured blocks now also share `ContentsLogicalLineLayoutSupport.js` for logical-line geometry.
  Live `positionToRectangle(...)` samples are mapped back into the delegate's own coordinate space before the flow
  caches gutter/logical-line Y, preventing block-local editor offsets from leaking into global gutter placement.
  The same samples may still expose visual-row width metadata, but structured minimap row ownership now comes from the
  parser-normalized block stream rather than from those sampled rows.
- `ContentsStructuredDocumentFlow.qml` no longer applies one global inter-block gap to text-family structured tags.
  Text-to-text flow for `paragraph`, `title`, `subTitle`, `eventTitle`, and the other prose-style text delegates now
  renders without synthetic bottom margin on `Enter`, while framed document blocks still keep explicit separation from
  surrounding prose.
- `ContentsEditorSelectionController.qml` owns inline style tag formatting and context-menu selection resolution only.
  It no longer owns markdown list shortcuts or generic key-event shortcut parsing.
- `ContentsEditorSessionController` no longer serializes note swaps behind pending-body save barriers. It keeps the
  live editor buffer authoritative, re-stages the old note into the buffered editor persistence controller, and allows
  the next selected note to bind immediately.
- `ContentsEditorSessionController` now also keeps local editor authority across same-note model echoes, so one successful
  echo cannot immediately reopen the door for the next stale polling snapshot to replace the current note body.
- `ContentsEditorSessionController` now also gates same-note model snapshot apply with an explicit typing-idle window
  (`typingIdleThresholdMs` + last local edit timestamp), so non-idle typing turns cannot be overwritten by an older
  snapshot captured before the latest user input.
- Desktop/mobile hosts now inject that idle threshold through `editorIdleSyncThresholdMs`, keeping the apply policy
  consistent across both surfaces.
- Desktop/mobile editor views now also gate timer-driven snapshot polling and deferred presentation commits on
  `typingSessionSyncProtected` plus `pendingBodySave`, not only focus state, so stale async snapshots cannot overwrite
  active typing when focus reporting briefly flaps.
- `ContentsDisplayView.qml` now delegates former host-owned policy roles to C++ coordinators and model-side
  controllers under `src/app/models/editor/display`, with narrow C++ ViewModel command surfaces under
  `src/app/viewmodel/editor/display`. The unified host keeps UI composition and repaint/focus execution only, while
  `ContentsDisplayHostModePolicy.qml` carries platform-mode presentation policy.
- `ContentsStructuredDocumentFlow.qml` now also converges structured document-host state through one
  `ContentsStructuredDocumentHost` instance and delegates collection normalization, focus resolution, and RAW mutation
  rules to dedicated C++ policy objects instead of keeping those host policies interleaved inside the QML flow object.
- Structured block text edits now check the delegate's expected RAW slice against the current document source before
  applying the splice.
  Mobile note dismiss/blur events that arrive from a stale block snapshot are ignored, preventing the same typed text
  from being inserted again during save/session handoff.
- `ContentsDocumentTextBlock.qml` now keeps paragraph boundary editing source-driven as well.
  Delegate-local key handling only emits split/merge intent; `ContentsStructuredDocumentMutationPolicy` owns the RAW
  rewrite so implicit prose lines and explicit `<paragraph>...</paragraph>` controllers obey the same rule set.
- Page/print view mode now also injects one explicit paper-palette flag from `ContentsDisplayView.qml` into both the
  whole-document HTML projection and the structured block delegate tree.
  Structured prose therefore no longer falls back to LVRS dark-theme body white while the paper surface itself stays
  white.
- `ContentsAgendaBlock.qml` and `ContentsCalloutBlock.qml` now also switch their native block chrome to a light
  paper-safe palette in page/print mode, replacing their previous hardcoded white text overrides with conditional
  dark-on-paper colors.
- That same structured document host now also emits selection-clear revisions plus one retained block hint, and
  `ContentsDocumentBlock.qml` forwards that cleanup into paragraph/callout/agenda delegates.
  Drag-selected text therefore no longer stays highlighted after the user activates another structured editor or clicks
  blank document space.
- Text block, agenda task, and callout cursor movement is routed through the host's cursor-only interaction path.
  That path advances cursor chrome without clearing the current native `TextEdit` selection, preserving desktop drag
  selection and iOS selection gestures.
- The input policy adapter now treats any focused body `TextEdit` as the native keyboard owner, not only mobile/iOS
  sessions. Ordinary editor shortcuts therefore stand down while typing so platform text-navigation and selection stay
  on the native `TextEdit` path. Explicit tag-management shortcuts remain available outside native composition so
  formatting commands can still write RAW inline style tags.
- `ContentsInlineFormatEditor.qml` does not install a live-text key handler for ordinary navigation or selection chords;
  those remain Qt/OS `TextEdit` behavior.
- Atomic structured blocks keep keyboard handling limited to plain navigation/delete and exact macOS Command Up/Down
  document-boundary movement.
- After that policy extraction, the flow and desktop/mobile hosts no longer keep dead duplicate QML helpers or
  pass-through import controllers that merely mirrored those dedicated collaborators.
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
- That same legacy diff path is no longer reachable from `ContentsDisplayView.qml`, because the host does not mount a
  whole-note inline editor or proxy object during note switching.
- `ContentsInlineFormatEditor.qml` now emits committed typing directly from the nested `TextEdit.onTextChanged` path
  whenever the change is not programmatic and IME composition has already settled.
  That keeps `ContentsEditorSessionController::editorText` moving with the visible buffer instead of leaving the note-open body
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
  - no `ContentsEditorSession.qml` compatibility wrapper remains
  - `ContentsEditorSessionController` owns pending/in-flight state, same-note RAW sync acceptance, and text
    normalization for persistence/model-sync turns
  - `ContentsEditorSelectionBridge` stays as the QML-facing adapter
  - `src/app/models/editor/persistence/ContentsEditorPersistenceController` owns the note-scoped buffered snapshot
    cache and recurring `1000ms` persistence drain clock
  - `ContentsNoteManagementCoordinator` serializes direct `.wsnote` writes plus open-count/stat follow-up work on the
    downstream management side
  - selection/typing controllers now default to immediate `.wsnbody` flush requests for live editor mutations, while
    the buffered persistence drain clock remains the retry/drain path for dirty note snapshots that were already
    accepted into the persistence controller
  - immediate flush calls now return `false` when the persistence lane rejects the current snapshot, so note-switch
    flows no longer treat a rejected enqueue as if the note had already been durably accepted
  - each successful queued write now also triggers one reconcile verify against filesystem RAW, so the visible
    note snapshot converges even when downstream body serialization canonicalizes markup/escaping.
  - same-note RAW/model snapshot rejection while the editor still has local-protection state no longer schedules a new
    persistence write from that rejection alone; the editor now waits for an explicit local mutation path instead of
    using model-sync refusal as a reverse write trigger
  - the save-path policy no longer depends on QML `undefined` checks or JavaScript regex normalization; those rules now
    execute in the typed C++ session controller
  - on Android SAF-mounted hubs, the successful note-management path now mirrors the locally written note package back
    into the original source document tree before the editor receives a final save success signal
  - when desktop/mobile hosts call `persistEditorTextImmediately(...)`, that path now issues one immediate persistence enqueue
    attempt for the current note payload instead of silently downgrading to deferred-only staging
- The RichText editor surface now decodes one safe-entity layer for display, so RAW-preserving source escapes like
  `&lt;` / `&gt;` / `&amp;` render as visible glyphs without changing the canonical note-body source contract.
- The print-page `Repeater` delegates now declare `required property int index`, removing a runtime `ReferenceError`
  class observed during live app execution.
- Gutter/minimap/editor default geometry now routes through LVRS `gap`, `stroke`, theme-color, and `scaleMetric(...)`
  tokens instead of scattered local editor pixel literals.
- The desktop gutter layout is now hard-clamped to its resolved token width, so markdown-list relayouts cannot make the
  gutter visibly squeeze or rebound while typing.
- Markdown list continuation on `Enter` is intentionally not owned by the editor input layer.
  The native `TextEdit` receives the newline and typed markdown marker text is persisted literally unless a
  tag-management rule applies.
- `ContentsEditorTypingController.qml` now also canonicalizes a standalone `---` typing line into the proprietary
  divider source token `</break>` before persistence.
- `ContentsRawBodyTagMutationSupport.js` now owns canonical RAW insertion payloads for generated agenda, callout,
  selected-range callout wrapping, divider, and prebuilt raw tag text. Keyboard/menu events still enter through the
  command surface, but QML hosts now apply helper-built `nextSourceText` payloads instead of assembling those body
  tags locally.
- Divider authoring shortcuts:
  - `Cmd+Shift+H` inserts canonical `</break>` into RAW at the current cursor
  - `Ctrl+Shift+H` fallback is also accepted when runtime Command mapping resolves as `ControlModifier`
- Agenda authoring shortcuts:
  - `Cmd+Opt+T` inserts canonical `<agenda date="YYYY-MM-DD"><task done="false"> </task></agenda>` (empty-body
    cursor anchor included)
  - `Ctrl+Alt+T` fallback is also accepted when runtime Command mapping resolves as `ControlModifier`
  - insertion now aborts unless that payload still validates as one complete `<agenda><task>...</task></agenda>` block
  - markdown-like `[] item` / `[x] item` lines are rewritten into agenda/task source blocks
  - pressing `Enter` inside `<task>` either creates the next `<task>` or exits agenda editing when the current task is
    empty
  - if agenda exit occurs on an empty task and all sibling tasks are empty, the entire agenda block is removed
- Callout authoring shortcuts:
  - `Cmd+Opt+C` wraps the active selected RAW text range as `<callout>...</callout>`
  - with no selection, `Cmd+Opt+C` inserts canonical `<callout> </callout>` (empty-body cursor anchor included) into
    RAW at the current cursor
  - `Ctrl+Alt+C` fallback is also accepted when runtime Command mapping resolves as `ControlModifier`
  - insertion now aborts unless that payload still validates as one complete `<callout>...</callout>` block
  - inside a callout, `Shift+Enter` inserts a callout body line break through native `TextEdit`
  - inside a callout, plain `Enter` closes the callout at the current cursor and moves editing outside the
    `</callout>` wrapper
  - inside an empty callout, plain `Backspace` deletes the entire callout RAW block
  - unhandled Enter/Backspace variants are explicitly marked unaccepted by the inline editor so native `TextEdit`
    behavior still runs for line breaks, ordinary deletion, and other OS-owned editing paths
- `ContentsStructuredCursorSupport.js` now centralizes plain-text cursor/source-offset mapping for agenda/callout block
  reparses, so local caret restoration survives entity-escaped RAW rewrites.
- `ContentsStructuredCursorSupport.js` now also carries inline-tag-aware text-block cursor/source mapping, letting
  structured text blocks edit RAW source spans directly from plain-text deltas instead of round-tripping rendered HTML
  through a DOM-to-source normalization step.
- `ContentsEditorDebugTrace.js` now centralizes verbose editor-domain QML tracing, and the desktop/mobile display
  hosts, session bridge, typing controller, structured document flow, and block controllers now emit lifecycle,
  mount/unmount, selection-sync, source-mutation, cursor/selection, and focus-transition logs through that shared helper
  instead of each file inventing a different console format.
- `ContentsStructuredDocumentFlow.qml` now limits itself to exposing the active delegate's local shortcut-insertion
  cursor hint.
  `ContentsStructuredDocumentHost` plus `ContentsStructuredDocumentFocusPolicy` own the actual structured
  shortcut/resource insertion-anchor resolution in C++.
- If the structured host temporarily has no interactive block, or a text-editable block has no live/pending caret
  anchor, that same insertion path no longer guesses with a block/document tail offset.
  Callers now use a live or pending source offset or fall back to the outer cursor bridge.
- That same structured-flow insertion bridge now also accepts dropped resource-tag batches, so resource imports inside
  agenda/callout/break notes insert next to the active block.
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
  snapshot on each keystroke; the unified host no longer mounts a second full-document editor instance behind the
  structured flow.
- The shared `contentEditor` compatibility reference now resolves to the structured document flow instead of a no-op
  inline editor proxy.
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
- Newly selected notes request the structured surface immediately; later same-note async reparses keep that structured
  surface mounted while agenda/callout parsing runs off the UI thread.
- Structured shortcut insertion now also resolves out of existing proprietary controllers before writing:
  - invoking agenda/callout insertion while the cursor is already inside an existing agenda/callout moves the new RAW
    block to the enclosing controller end first
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
- Agenda parsing and agenda-internal source-mutation backend logic used by those shortcuts now lives in
  `src/app/models/editor/tags/ContentsAgendaBackend.*`, while direct shortcut source splices live in
  `src/app/models/editor/tags/ContentsRawBodyTagMutationSupport.js`.
- Callout parsing and plain-Enter exit backend logic now lives in
  `src/app/models/editor/tags/ContentsCalloutBackend.*`, while direct shortcut source splices live in
  `src/app/models/editor/tags/ContentsRawBodyTagMutationSupport.js`.
- Agenda/callout render-model projection now lives in
  `src/app/models/editor/renderer/ContentsStructuredBlockRenderer.*`, so QML overlay layers consume renderer-owned data
  instead of calling parse backends directly.
- `ContentsEditorSessionController` now treats `date="yyyy-mm-dd"` as a modification-time placeholder:
  - when local note modification is staged for persistence, placeholder dates are rewritten to current `YYYY-MM-DD`
  - passive same-note model sync does not rewrite agenda dates
- `ContentsEditorSessionController` now also normalizes empty structured blocks into one-space anchors on model sync:
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
- Empty markdown list items are not post-processed by custom editor input code; ordinary newline and deletion behavior
  stays native.
- `Page` / `Print` now mount the live RichText editor inside an outer paper-document viewport, so the paper grows with
  the note instead of remaining a fixed-height scaffold.
- `Page` / `Print` paper visuals now use an A4-style off-white sheet gradient with per-page separators and subtle
  shadow depth, replacing the prior plain-white flat backdrop.
- `Page` / `Print` mode gating plus paper geometry/page-count math are now provided by
  `ContentsPagePrintLayoutRenderer` in `src/app/models/editor/renderer`, so editor QML hosts consume backend layout state.
- The repository no longer operates scripted editor tests; the per-file regression notes in this directory are
  documentation-only behavior contracts.
- The remaining automated regression surface is the C++ build/runtime suite under `test/cpp/`; the structured-flow
  caret persistence and live-caret shortcut notes in this directory remain documentation-only behavior contracts.
- Cursor restoration for ordinary typing/focus recovery now routes through the controller-level cursor setter instead of
  rewriting `cursorPosition` into the controller, `editorItem`, and `inputItem` together.
- The shared editor controller no longer depends on its own synthetic IME commit queue for ordinary typing, so a just-typed
  word is no longer left behind a controller-local composition flag while the user immediately clicks to move the cursor.
- The shared editor controller now also prefers the native `QtQuick.TextEdit` edited-signal / input-method commit path
  instead of maintaining its own synthetic IME commit flag, aligning Hangul composition behavior with standard text
  editors and word processors more closely.
- That same controller now also suppresses one echoed native `textEdited` turn for host-driven RichText surface
  replacement, preventing debounced presentation refresh from re-entering the RAW typing-mutation path as a fake user
  edit.
- The shared editor controller now also normalizes tab indentation width through `TextEdit.tabStopDistance` using runtime
  font metrics for four spaces, so Tab indent depth no longer jumps to an oversized default column.
- The shared editor controller no longer intercepts direct `Tab` key insertion. Tab handling stays with the native
  `TextEdit` path so keyboard traversal, repeat, and platform text editing policy are not overridden by QML.
- The shared editor controller also rejects focused stale host text echo in native-input mode after a local edit.
  Structured text, agenda, and callout delegates keep live previous-source/text snapshots so rapid iOS input can rebase
  before the parser publishes a newer block snapshot.
- Legacy cursor restoration and resource-import surface restoration now pause on `inputMethodComposing`/`preeditText`
  instead of writing through a live platform IME session.
- `ContentsEditorTypingController.qml` no longer drops a committed `textEdited` mutation only because a transient
  model-sync guard bit is still set; committed user typing now always refreshes local authority and persistence staging.
- The shared selection controller now also primes right-click context-menu selections from the desktop
  command-surface `MouseArea` press, so multi-block or mixed-inline dragged selections survive the menu-opening click
  instead of collapsing to one fragment before formatting.
- Shared inline-format actions now resolve the live selection back into RAW boundaries and insert proprietary
  opening/closing tags directly into `.wsnbody`, so formatting stays governed by authoritative source text instead of
  temporary RichText fragment splits.
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
