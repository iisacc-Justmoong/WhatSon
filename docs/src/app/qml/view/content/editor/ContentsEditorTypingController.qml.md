# `src/app/qml/view/content/editor/ContentsEditorTypingController.qml`

## Responsibility

`ContentsEditorTypingController.qml` owns ordinary editor typing mutations for `ContentsDisplayView.qml`.

This controller exists to keep plain typing separate from inline-format application:
- ordinary typing/backspace/delete/enter/paste
- no whole-document RichText normalization
- direct raw `.wsnbody` source replacement only

## Mutation Model

- Seeds its authoritative plain-text snapshot and logical-to-source offset table from
  `ContentsLogicalTextBridge.logicalText` plus `logicalToSourceOffsets()` only when the host editor view commits a
  new `documentPresentationSourceText` snapshot.
- When it must fall back to `view.selectedNoteBodyText`, it now only accepts that payload if
  `view.selectedNoteBodyNoteId == view.selectedNoteId`.
  The typing controller therefore no longer treats a stray selected-body string as authoritative plain text without an
  explicit ownership match.
- Between those presentation commits, it keeps an incremental live cache of:
  - the previous authoritative plain text
  - logical line-start offsets
  - the logical-to-source offset table needed to splice `.wsnbody`
- Reads the live edited plain text from `contentEditor.currentPlainText()` when the wrapper exposes it, falling back to
  `contentEditor.getText(0, length)` only for legacy editor surfaces.
- The controller now treats that plain-text delta path as the only authoritative edit path for ordinary typing.
  It no longer reverse-normalizes the whole RichText surface back into RAW source during same-note typing because the
  rendered agenda/callout surface is intentionally lossy and can collapse the note back to the note-open session
  snapshot.
- Receives committed edit notifications only after the native `TextEdit` input-method session settles, so diffing is
  based on stable plain-text snapshots instead of transient Hangul/Japanese preedit fragments.
- Programmatic RichText surface replacement from the host wrapper is no longer allowed to re-enter this path as a fake
  committed edit notification; only real post-sync user edits should reach this controller.
- The controller now explicitly exits early while the host marks
  `view.programmaticEditorSurfaceSyncActive == true` or while `editorSession.syncingEditorTextFromModel` is still
  raised for the current model-driven body sync.
  Resource/body presentation rebuilds therefore cannot be diffed as if the user had typed the placeholder surface.
- The controller now also exits early whenever `editorSession.editorBoundNoteId != view.selectedNoteId`.
  Once note selection has moved on, the typing controller must not reinterpret the previously bound note's editor
  surface as a fresh edit for another note.
- The controller now also exits early while `view.resourceDropEditorSurfaceGuardActive == true`.
  External file drops can therefore import/link `.wsresource` packages without letting a same-turn native `TextEdit`
  drop mutation serialize the visible RichText surface back into `.wsnbody` as escaped attribute text.
- The controller now also rejects plain-text deltas that collapse onto a zero-width RAW source span or that touch an
  existing `<resource ... />` token at the source layer.
  This protects inline resource placeholders from being reserialized as partial escaped tail fragments when the
  RichText surface reports a lossy post-render delta.
- Legacy inline-editor mutation paths now also reject any candidate RAW rewrite whose `<resource ... />` count is lower
  than the current source snapshot while `view.showStructuredDocumentFlow == false`.
  If a stale RichText/plain-text surface delta, delete key path, or explicit plain-text rewrite would silently drop a
  resource tag, the controller restores the editor surface from authoritative RAW presentation instead of persisting
  the damaged source.
- Collapsed-caret insertion now also advances past any consecutive closing inline-style tags that sit exactly at the
  mapped RAW boundary.
  A visible caret placed after a bold/italic/underline/strikethrough/highlight run therefore resolves to the source
  offset after the corresponding closing tags rather than to the interior edge before them.
- The controller now also owns tag-aware raw deletion before `TextEdit` default handling runs.
  Backspace/Delete no longer has to land as one-character plain-text diffs first and then hope the source bridge can
  reconstruct the intended tag boundary afterward.
- Computes a single contiguous replacement delta (`start`, `previousEnd`, `insertedText`) from those two plain-text
  projections.
- When that delta is a single `Enter` insertion on a markdown list line, the controller now expands the inserted text
  before persistence.
- After list-continuation preprocessing, if the edited logical line becomes exactly `---`, the controller rewrites that
  whole line into the canonical divider source token `</break>` before source persistence.
- The controller now also owns agenda source shortcuts:
  - `queueAgendaShortcutInsertion()` inserts canonical agenda source directly into RAW at the current cursor.
  - empty agenda shortcut payloads include an internal one-space task-body anchor so cursor/Enter flow can still
    address the inserted task in logical/source mapping.
  - before insertion, the controller now also validates that the payload is still a complete
    `<agenda ...><task ...>...</task></agenda>` block and aborts instead of writing partial fragments
  - typing `[] item` or `[x] item` triggers todo shorthand canonicalization.
  - pressing `Enter` inside `<task>` triggers agenda-aware newline handling.
- The controller now also owns callout source shortcuts:
  - `queueCalloutShortcutInsertion()` inserts canonical callout source directly into RAW at the current cursor.
  - empty callout shortcut payloads include an internal one-space body anchor so cursor/Enter flow can still
    address the inserted callout in logical/source mapping.
  - before insertion, the controller now also validates that the payload is still a complete
    `<callout>...</callout>` block and aborts instead of writing partial fragments
  - pressing `Enter` on a trailing empty callout line exits callout editing instead of adding another empty inner line.
- The controller now also owns divider source shortcuts:
  - `queueBreakShortcutInsertion()` inserts canonical `</break>` source directly into RAW at the current cursor.
  - insertion is routed through the same line-boundary normalization used by other raw-structure shortcuts.
- The same raw-source insertion helper is now also reused by editor resource-drop linking:
  - `insertRawSourceTextAtCursor(...)` resolves the live logical caret back into a RAW `.wsnbody` source offset
  - newline padding is normalized around the inserted RAW fragment
  - collapsed-caret insertions also skip immediately adjacent closing inline-style tags before splicing source
  - local-authority marking, cursor restore, same-note persistence, and host `editorTextEdited(...)` signaling stay on
    the exact same contract already used by agenda/callout/break shortcut insertion
- Plain `Enter` / `Return` on the legacy inline editor now also has a RAW-first path:
  - `handlePlainEnterKeyPress(event)` intercepts unmodified Enter before Qt RichText mutates the live document on its
    own
  - the controller replaces the current logical selection/caret with `\n` directly against RAW source, then commits
    that source mutation through the same same-note persistence/surface-refresh contract used elsewhere
  - list continuation, divider rewrite, agenda task exit, and callout exit still reuse the existing source-aware enter
    transforms on top of that explicit newline replacement
  - IME composition turns are excluded so Enter can still finalize the platform input-method session first
- Backspace/Delete now also has a tag-aware raw-source fast path:
  - when the pending delete span intersects an XML-like token such as `<resource ... />`, `<callout>`, `</callout>`,
    `<agenda ...>`, `</agenda>`, inline-style tags, or comment-like editor markers, the controller expands the delete
    range to the full token boundary before mutating `.wsnbody`
  - caret-only Backspace removes the whole tag immediately behind the source cursor, and caret-only Delete removes the
    whole tag immediately at or under the source cursor
  - selection delete likewise expands to cover any partially intersected tag tokens so a raw source edit cannot leave
    half-deleted tag fragments behind
- Structured shortcut insertion now resolves the actual RAW insertion point before writing:
  - if the logical cursor is already inside an existing `<agenda>...</agenda>` or `<callout>...</callout>`, the new
    shortcut block is moved to the end of that enclosing block instead of being nested into it
  - newline padding is then applied around that resolved outer insertion point, so the new block becomes a standalone
    RAW block instead of continuing the surrounding line
- Agenda-specific source mutation logic above is delegated to `ContentsAgendaBackend`:
  - `buildAgendaInsertionPayload(...)`
  - `detectTodoShortcutReplacement(...)`
  - `detectAgendaTaskEnterReplacement(...)`
- Callout insertion payload generation is delegated to `ContentsCalloutBackend.buildCalloutInsertionPayload(...)`.
- Callout enter-exit detection is delegated to
  `ContentsCalloutBackend.detectCalloutEnterReplacement(...)`.
- The divider shortcut rewrite is line-scoped:
  - it replaces the whole current line range, not only the last typed `-`
  - it does not trigger when `---` appears as part of a longer token (`abc---`, `----`, `---text`)
- The continuation path is tolerant of short source-sync lag: when fast typing causes the committed delta to include the
  last few typed characters plus the trailing newline together, the controller still treats that edit as one list
  continuation candidate instead of requiring `insertedText === "\n"` exactly.
- The continuation path is also tolerant of first-`Enter` line replacement batches: if the editor commits the whole
  current list line plus the trailing newline before `view.editorText` catches up, the controller still accepts that
  edit as a continuation candidate instead of requiring a pure insertion-only delta.
- The continuation check is source-driven, not renderer-driven:
  - it maps the logical cursor offset back into `view.editorText`
  - it inspects the source-side line prefix there, so `-`, `*`, `+`, and ordered markers still continue correctly even
    though the RichText editor may display unordered markers as a bullet glyph (`•`)
- The continued marker is then written back into source before persistence:
  - unordered list lines (`- ` / `* ` / `+ `) continue with the same marker and indentation
  - ordered list lines (`1. ` / `2) `) continue with the next number, same delimiter, and same indentation
  - ordered list continuation is tolerant of missing post-marker whitespace (`1.text`, `1)text`) and normalizes the
    continued prefix back to a spaced marker (`2. ` / `2) `)
  - numeric literals without a separator such as `3.14` are still excluded from list continuation
  - delayed `Enter` commits that arrive as `item\n` instead of bare `\n` still append the continued marker after that
    newline
  - if the unordered list line is re-sent from the rendered editor with a leading bullet glyph (`• item\n`), the
    controller rewrites that batch back to a source marker (`- item\n`, `* item\n`, `+ item\n`) before persistence so
    the list source does not collapse into display glyphs
  - marker-only empty list lines do not auto-continue; pressing `Enter` on that empty continued list item now removes
    the list marker and leaves a plain blank line, breaking the repeated list-continuation chain
- Pressing plain Enter in the middle of prose that already has a following line must therefore split the line without
  deleting the next line's text; the next-line suffix remains in RAW and only the newline boundary changes.
- Resolves the delta back into source offsets through the incremental live offset cache first, only reseeding from
  `ContentsLogicalTextBridge` when the presentation snapshot is refreshed.
- After each accepted typing mutation, the controller now pushes that incremental logical-text / line-start /
  logical-to-source state back into `ContentsLogicalTextBridge` through `adoptIncrementalState(...)`, so gutter,
  selection, and cursor helpers do not need a whole-note bridge rebuild to stay current while the user is typing.
- That bridge-adoption step now also advances the controller's own `liveSnapshotSourceText` to the just-written RAW
  source snapshot.
- While `editorSession.localEditorAuthority` is still active and `documentPresentationSourceText` lags behind the live
  RAW `editorText`, the controller now keeps trusting that incremental live cache instead of reseeding from the stale
  presentation snapshot.
  This prevents one deferred RichText/presentation refresh from rolling the typing diff base back to an older note
  snapshot and duplicating already committed prose into RAW.
- Line-start offsets are now rebuilt from the post-edit logical text on every accepted mutation instead of splice-
  merging prior offsets.
  This keeps line-count shrink cases (line-wrap collapse, newline deletion, paragraph merge) from leaving stale line
  starts behind in gutter/minimap geometry.
- Delegates the final source splice to `ContentsTextFormatRenderer.applyPlainTextReplacementToSource(...)`.
- List-continuation cursor restoration now also waits for IME preedit to finish and then moves the cursor through the
  wrapper-level cursor setter only.
  The controller no longer writes the same logical cursor offset into the wrapper, `editorItem`, and `inputItem`
  in parallel.

This means ordinary typing stays on the logical plain-text/source-offset bridge and does not round-trip the rendered
RichText surface through `normalizeEditorSurfaceTextToSource(...)`, so fragment scaffold/comment markup and lossy
structured-block projection do not re-enter the normal typing path.

## Persistence Rules

- After source replacement, the controller updates `view.editorText`.
- If list continuation inserted extra markdown marker text, the controller also restores the live cursor to the
  continued item start after programmatic surface sync.
- If the divider shortcut rewrote `---` to `</break>`, the controller restores the cursor near the rewritten divider
  line after the programmatic source sync.
- It marks local editor authority before save, matching the existing editor session contract.
- A committed `textEdited` turn is now always treated as user-authoritative typing input.
  The controller no longer drops that turn only because `syncingEditorTextFromModel` happened to still be true from a
  previous model-sync tick.
- Committed typing now always requests `view.persistEditorTextImmediately(...)` for the rewritten RAW note body.
- The controller no longer exposes or honors a host-level deferred-persistence escape hatch for ordinary typing.
- Any later retry or dirty-snapshot draining remains owned by the downstream bridge/idle-sync controller after that
  immediate flush request is accepted.
- The host view still emits `editorTextEdited(...)` for the broader editor-shell lifecycle.

## Normalization Rules

- Plain-text comparison normalizes:
  - CRLF / CR -> LF
  - `U+2028` / `U+2029` -> LF
  - RichText object-replacement glyphs (`U+FFFC`) -> removed
  - NBSP -> regular space
- That object-replacement normalization lets the live RichText surface contain real inline image/resource objects while
  the typing diff path still compares only canonical logical `.wsnbody` text.
- The source-side splice path escapes literal text before inserting it into `.wsnbody`, so ordinary typing cannot
  inject raw inline tags accidentally.
- Markdown-style prefixes such as `1. `, `- `, `# `, `> `, and `` ``` `` still enter the source as literal typed text;
  the renderer layer is responsible for their RichText presentation after the source update lands.
- The only typing-side markdown shortcut exception is a standalone `---` line, which is canonicalized to `</break>`
  before it reaches source persistence.
- Agenda shortcuts are the other source-side exception set:
  - `[]`/`[x]` todo shorthand is canonicalized into proprietary `<agenda>/<task>` source tags
  - agenda insertion always writes `date` as `YYYY-MM-DD`
  - task insertion always writes canonical `done="true|false"`
  - canonicalization rules are centralized in `src/app/agenda/ContentsAgendaBackend.cpp`
- Callout insertion is another source-side shortcut exception:
  - `Cmd+Opt+C` inserts canonical `<callout>...</callout>` source tags at the current cursor
  - `Ctrl+Alt+C` fallback is also accepted when runtime Command mapping resolves as `ControlModifier`
  - pressing `Enter` twice at the end of a callout exits the wrapper on the second `Enter`
  - canonical insertion/exit payload generation is centralized in `src/app/callout/ContentsCalloutBackend.cpp`
- Divider insertion is another source-side shortcut exception:
  - `Cmd+Shift+H` inserts canonical `</break>` source tags at the current cursor
  - `Ctrl+Shift+H` fallback is also accepted when runtime Command mapping resolves as `ControlModifier`
- Resource drop linking is now another raw-source insertion exception:
  - dropped files are imported first, then the editor injects canonical self-closing `<resource ... />` RAW source at
    the live logical caret by routing through `insertRawSourceTextAtCursor(...)`
  - this path no longer splices by raw `TextEdit.cursorPosition`, which could drift from `.wsnbody` offsets once the
    note already contained structured or resource tags
  - later typing around those inline RichText image blocks still ignores the editor's internal object glyphs, so one
    ordinary keystroke does not look like a whole-resource plain-text mutation
  - if the RichText surface later reports a replacement that resolves only to the virtual placeholder span for that
    resource block, the controller now drops that replacement and restores the editor surface from authoritative RAW
    source instead of persisting a broken resource-tail fragment
  - if a legacy inline-editor rewrite would reduce the total number of canonical `<resource ... />` tokens in RAW, the
    controller now treats that rewrite as invalid and restores the surface from the last authoritative RAW projection
    instead of letting the resource tag disappear

## Regression Checks

- Markdown-list continuation is now tracked as a documented behavior contract only; this repository no longer maintains
  a dedicated scripted test for it.
- Typing ordinary letters should update raw `.wsnbody` without serializing the whole RichText document.
- Typing with the visible caret immediately after `<bold>...</bold>`, `<italic>...</italic>`, `<underline>...</underline>`,
  `<strikethrough>...</strikethrough>`, or `<highlight>...</highlight>` must keep the next inserted text outside that
  styled run instead of splicing it before the closing tag.
- A committed delete/backspace turn must update the session through the plain-text/source-offset bridge itself; the
  controller must not depend on whole-surface RichText reserialization to rescue the edit.
- Backspace/Delete while the logical/source cursor touches `<resource ... />`, `<callout>`, `</callout>`,
  `<agenda ...>`, `</agenda>`, or inline-style tags must remove the full tag token, not only one source character.
- A selection delete that partially covers a source tag must expand to the tag boundary before persistence; `.wsnbody`
  must not retain broken tail fragments such as `resource ... />`, `/callout>`, or `&quot;image&quot; ... /&gt;`.
- The typing controller must not treat `selectedNoteBodyText` as authoritative unless the host also proves that the
  body payload belongs to the currently selected note.
- A host-driven RichText/resource presentation refresh must not re-enter this controller while
  `programmaticEditorSurfaceSyncActive` is raised, even if the nested `TextEdit` emits a late fallback
  `textEdited(...)` notification for the superseded surface.
- A file-drop import/link turn must not re-enter this controller while `resourceDropEditorSurfaceGuardActive` is
  raised, even if Qt mutates the nested `TextEdit` surface as part of native drop handling.
- Typing ordinary letters must not require a full `ContentsLogicalTextBridge` rebuild or full logical/source offset-map
  regeneration on every committed keystroke.
- Typing ordinary letters should keep `ContentsLogicalTextBridge.logicalLineCount`, line-start offsets, and
  logical-to-source offsets current through incremental adoption instead of waiting for a whole-note bridge refresh.
- Typing a single newline insertion or deletion must keep `logicalLineStartOffsets` and `logicalLineCount` exact in
  both growth and shrink directions; deleting a line break must reduce line count immediately without leaving stale
  extra gutter lines.
- `Space`, `Enter`, `Backspace`, and selection replacement should update the correct source span.
- Typing inside notes that already contain `<agenda>`, `<task>`, `<callout>`, or `</break>` must not rebuild RAW by
  reserializing the rendered RichText surface, because that surface omits proprietary wrapper tags by design.
- Typing inside notes that already contain inline RichText image/resource blocks must not treat those rendered document
  objects as inserted plain-text characters during diffing.
- A plain-text delta that resolves to the same RAW source offset for both its logical start and end must be treated as
  a virtual placeholder mismatch, not as a real `.wsnbody` replacement candidate.
- Direct typing must not leak fragment comment markup such as `<!--StartFragment-->`.
- Typing literal `<bold>` text should persist as literal text, not as an executable inline tag.
- Hangul IME composition must mutate `.wsnbody` only once per committed syllable/result, not once per preedit step.
- The typing controller must not depend on wrapper-owned synthetic IME commit flags; it should only react to the
  native `TextEdit` state that survives after composition settles.
- A user `textEdited` event must not skip local-authority marking/persistence staging solely due to a transient
  model-sync guard bit from a prior same-note sync cycle.
- Post-`Enter` list-continuation cursor restore must not land on the wrong logical offset because multiple nested
  cursor objects were rewritten out of order.
- Committed typing must not route through a host-level deferred-persistence flag; the default contract is now
  write-through `.wsnbody` sync.
- Typing `- item` or `1. item` should persist the literal markdown marker text in source rather than an already-expanded
  bullet/number glyph representation.
- Pressing `Enter` on an unordered markdown list that is visually rendered with a bullet glyph in the editor must still
  continue with the original source marker (`-`, `*`, or `+`) instead of degrading to a bare newline.
- Pressing `Enter` at the end or middle of a non-empty markdown bullet line should persist `\n- ` / `\n* ` / `\n+ `
  continuation text instead of a bare newline.
- Pressing `Enter` on an empty continued unordered list item (`- ` / `* ` / `+ ` / `• ` with no body text) should
  remove that list marker and leave a plain blank line instead of creating another empty list continuation line.
- Pressing `Enter` immediately after rapid typing on a non-empty markdown list line must still continue that list even
  when the committed edit batch includes both the final typed characters and the newline.
- Pressing the first `Enter` on a newly typed unordered markdown line must still persist a real source marker and the
  continued next marker even when the committed edit batch contains the whole logical line plus newline.
- Leaving a note and reopening it after same-note typing must not blank the editor just because the RichText
  presentation cache had not caught up to the latest RAW snapshot yet.
- Ordinary prose typing must not duplicate previously committed text into RAW when `documentPresentationSourceText`
  is one or more turns behind the live `editorText`.
- Pressing `Enter` on a non-empty ordered markdown list line should persist the incremented next prefix (`1.` -> `2.`,
  `3)` -> `4)`).
- Pressing `Enter` on an empty continued ordered markdown list item (`1. ` / `1) ` with no body text) should remove the
  ordered marker and leave a plain blank line instead of extending the empty list.
- Pressing `Enter` on a non-empty ordered line such as `1.hello` or `1)항목` should also continue the list even when
  the original source omitted the usual separator space.
- Typing a standalone `---` line must immediately persist canonical source `</break>` (not literal `---`).
- Typing `---` inside longer text (`abc---`, `---todo`, `----`) must not trigger divider canonicalization.
- `Cmd+Opt+T` must insert one canonical agenda block at the current cursor into RAW source, and empty task bodies must
  still remain cursor-reachable via an internal one-space anchor.
- `Ctrl+Alt+T` fallback must trigger the same agenda insertion behavior when runtime Command mapping resolves as
  `ControlModifier`.
- `Cmd+Opt+C` must insert one canonical `<callout>...</callout>` wrapper at the current cursor into RAW source, and
  empty callout bodies must still remain cursor-reachable via an internal one-space anchor.
- `Ctrl+Alt+C` fallback must trigger the same callout insertion behavior when runtime Command mapping resolves as
  `ControlModifier`.
- `Cmd+Shift+H` must insert one canonical `</break>` token at the current cursor into RAW source.
- `Ctrl+Shift+H` fallback must trigger the same divider insertion behavior when runtime Command mapping resolves as
  `ControlModifier`.
- Triggering a new agenda/callout shortcut while the cursor already sits inside an existing agenda/callout must place
  the new block after the enclosing closing tag, not inside the existing block body.
- Dropping a file into a note that already contains earlier proprietary tags must still insert the new
  `<resource ... />` at the correct RAW source position instead of counting visible plain-text cursor offsets as direct
  `.wsnbody` offsets.
- Pressing `Enter` twice at the end of `<callout>...</callout>` must move the cursor out of the callout on the second
  `Enter` and must not duplicate closing `</callout>` tags.
- Typing `[] task` must persist canonical `<agenda date="..."><task done="false">task</task></agenda>`.
- Typing `[x] task` must persist canonical `<agenda date="..."><task done="true">task</task></agenda>`.
- Pressing `Enter` in a non-empty `<task>` must create the next `<task done="false">` sibling block.
- Pressing `Enter` inside an empty `<task>` must exit agenda editing rather than stacking additional empty task blocks.
- Pressing `Enter` inside an empty `<task>` and leaving an agenda where all tasks are empty must remove the entire
  `<agenda>...</agenda>` block from source.
