# `src/app/qml/view/content/editor/MobileContentsDisplayView.qml`

## Responsibility
Mobile content editor host.

## Structured Document Flow
- `agenda`, `callout`, `resource`, and `break` remain `.wsnbody` body tags, but mobile now limits structured-flow
  activation to non-resource block types.
  A note that only gained `<resource ... />` tags must stay on the legacy editor surface so the active editor
  implementation does not swap mid-edit.
- Canonical live `<resource ... />` markup is still tracked through the live editor buffer, presentation snapshot, and
  selection-bridge body-source snapshot so `ContentsBodyResourceRenderer` can resolve against the freshest RAW source,
  but that resource presence alone no longer activates `ContentsStructuredDocumentFlow.qml`.
- `ContentsStructuredDocumentFlow.qml` therefore remains the shared block-flow host for agenda/callout/break notes and
  for notes that already entered structured-flow mode for some non-resource reason.
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
- The dedicated full-surface `ContentsResourceViewer` is now reserved for direct resource-package browsing from the
  Resources hierarchy only.
  Notes opened from other hierarchies must remain on the ordinary note editor surface even when their body contains
  inline `<resource ... />` tags.
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
- Mobile now also rescans the live `editorText` buffer for canonical `<resource ... />` markup on each edit turn.
  That keeps resource resolution tied to the newest RAW source without forcing a same-turn host swap for resource-only
  notes.
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
- The current default mobile editable note surface also keeps RichText inline-image upgrading disabled.
  Resolved bitmap resources therefore stay on the same parser-owned placeholder/overlay path as desktop:
  - the editor surface keeps the RAW-derived logical placeholder slot
  - `ContentsResourceLayer.qml` paints the visible centered image frame at that authored body position
  Follow-up typing after `<resource ... />` no longer depends on a mutable RichText image object living inside the
  editor document.
- `ContentsBodyResourceRenderer` on mobile now also receives `libraryHierarchyViewModel` as a fallback note-directory
  resolver so body resource rendering survives hierarchy-switch lag and active domains that do not expose
  `noteDirectoryPathForNoteId(QString)`.
- RichText dirtiness checks now compare against the already-upgraded inline-resource HTML instead of the unresolved
  placeholder payload, preventing resource-bearing notes from staying permanently dirty while the RichText surface is
  active.
- `ContentsResourceLayer.qml` remains only for resource types that are not upgraded into RichText-inline media blocks
  yet, or for native/plain-input mobile routes that are not using the RichText editor surface projection.
  On the current default mobile route, bitmap images still use the source-aligned layer while non-image resources do
  the same.
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
