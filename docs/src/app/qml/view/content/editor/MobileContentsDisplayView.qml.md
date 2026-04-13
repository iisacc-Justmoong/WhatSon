# `src/app/qml/view/content/editor/MobileContentsDisplayView.qml`

## Responsibility
Mobile content editor host.

## Structured Document Flow
- `agenda`, `callout`, `resource`, and `break` remain `.wsnbody` body tags, but mobile now only promotes the note into
  the structured document-flow surface when that note also owns at least one resolved body resource slot.
- Mobile now gates `structuredDocumentFlowEnabled` off `bodyResourceRenderer.resourceCount > 0`, so plain notes and
  non-resource structured tags stay on the legacy editor path while inline-resource notes switch into the shared block
  flow.
- `ContentsStructuredDocumentFlow.qml` is therefore active for resource-bearing notes as well, so mobile uses the same
  body-owned QML block path as desktop instead of leaving inline resources to a detached overlay.
- Structured block rewrites route through `applyDocumentSourceMutation(...)` so mobile keeps the same RAW persistence
  contract as desktop, but they no longer force an immediate full legacy presentation rebuild while structured-flow
  editing remains active.
- While structured-flow mode is active, mobile also disconnects the hidden legacy agenda/callout overlay models so note
  open does not pay for both rendering paths at once.
- The same structured-flow surface now also receives `bodyResourceRenderer.renderedResources`, letting mobile resource
  blocks resolve from `<resource ... />` to the actual asset file inside the referenced `.wsresource` package before
  `ContentsResourceRenderCard` paints the inline frame.
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
- Mobile now also carries the same `whatson-resource-block` upgrade helper as desktop.
- Once the selected note has entered structured-flow mode, mobile now prefers `ContentsResourceBlock.qml` in the shared
  structured document host for inline resource display instead of depending on the RichText placeholder upgrade path.
- In structured-flow mode, `ContentsBodyResourceRenderer` now follows the live `editorText` source directly so resource
  blocks can refresh against the current RAW note text without waiting for the legacy presentation snapshot.
- When mobile later enters the RichText projection path, resolved image resources can be rewritten into real RichText
  `<img>` paragraphs using the renderer-resolved resource URL, without the old trailing blank placeholder paragraphs.
- The current default mobile native-input path still keeps actual image paint on the source-aligned resource layer.
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
- The shared minimap slot now also keeps a fixed layout width while visible.
  This matches the desktop contract and prevents sibling editor content from collapsing the minimap column to zero
  width if the mobile host ever enables that rail.
- The mobile editor host likewise pins its inner `RowLayout` to `Qt.LeftToRight`, so the shared gutter/editor/minimap
  column order cannot flip under inherited layout-direction changes.
- Mobile now also drives `ContentsInlineFormatEditor.blockExternalDropMutation` from
  `resourceDropActive || resourceDropEditorSurfaceGuardActive`, so the nested `TextEdit` turns read-only as soon as a
  valid external file drag is hovering over the editor and stays frozen until the dedicated resource-drop turn
  finishes.
- Mobile drop handling now also prefers `drop.acceptProposedAction()` when the Qt drop event exposes it, then sets
  `drop.accepted = inserted`, keeping file import/linking on the dedicated resource-drop path instead of leaving the
  nested editor free to reinterpret the drop as editable text content.
