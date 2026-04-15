# `src/app/qml/view/content/editor/ContentsInlineFormatEditor.qml`

## Responsibility

`ContentsInlineFormatEditor.qml` is the dedicated RichText input surface for `.wsnbody` inline-format editing.

It keeps the editor contract expected by `ContentsDisplayView.qml` (`cursorPosition`, `selectionStart`,
`selectionEnd`, `selectedText`, `contentHeight`, `editorItem`, `inputItem`, `positionToRectangle(...)`) while using a
plain `QtQuick.TextEdit` as the actual rendering and input engine.

- The repository does not route note editing through an LVRS-provided `LV.TextEditor` anymore.
- Performance-sensitive editor work should therefore target this wrapper and the underlying `QtQuick.TextEdit`
  lifecycle directly.

## Key Behavior

- Accepts `renderedEditorText` as an input-only property (`text`) and syncs it into the underlying `TextEdit`
  programmatically.
- Hosts now treat that `renderedEditorText` payload as a debounced presentation cache rather than a per-keystroke
  whole-document mirror, so the wrapper is no longer asked to absorb a full RichText surface replacement on every
  ordinary typing tick.
- The wrapper now also mirrors editor-state changes back to `Qt.inputMethod.update(...)`:
  - cursor / selection changes emit `Qt.ImQueryInput`
  - cursor-geometry / clip changes emit cursor-rectangle queries including `Qt.ImAnchorRectangle`
  - this follows Qt's documented mobile-text-selection contract, where selection handles and similar platform features
    are surfaced through `QInputMethod`
- When the wrapper must reapply an existing selection after a programmatic text sync, it now prefers
  `TextEdit.moveCursorSelection(...)` over `select(start, end)` so the active cursor edge is preserved together with
  the selected range.
- The visible caret now follows the wrapper's full `focused` contract instead of the narrower FocusScope-only
  `activeFocus` flag.
  If the nested `TextEdit` still owns input focus after one host-side surface sync, the caret therefore stays visible
  instead of disappearing until the wrapper FocusScope also becomes active again.
- On native-input mobile paths the wrapper also adds a passive touch `TapHandler` on top of `TextEdit`:
  - when the software keyboard is hidden, touch `press` no longer immediately focuses `TextEdit` or moves selection
  - a single tap (`tapCount === 1`) now activates input at the tapped cursor position and explicitly requests keyboard
    show
  - double-tap reselects the touched word via `TextEdit.selectWord()`
  - triple-tap expands to the surrounding newline-delimited paragraph
  - the resulting programmatic selection still reuses the same input-method update path so iOS selection UI stays in
    sync
- In native-input mobile mode, `activeFocusOnPress` and `selectByMouse` are now gated by keyboard visibility:
  - keyboard hidden: `press`/drag path stays scroll-first and non-editing
  - keyboard visible: cursor placement/selection behavior remains enabled
- Hosts can opt into `preferNativeInputHandling`:
  - while the editor keeps focus or an IME preedit session is active, app-driven `text` resync is deferred
  - the latest deferred payload is flushed after focus leaves or when no native input session is active anymore
  - this lets mobile keep OS-native composition, double-tap selection, and repeat-backspace behavior ahead of the
    app's RichText surface normalization
- IME preedit/composition now always outranks app-driven surface resync even on the desktop RichText host:
  - any active `inputMethodComposing` / `preeditText` session defers `text` reinjection
  - this prevents Hangul jamo assembly or backspace from being interrupted by a late programmatic note-body refresh
- The same desktop RichText path now also falls back to a temporary PlainText input surface while IME preedit is
  active.
  During that fallback the wrapper snapshots the current visible plain text into the nested `TextEdit`, allowing the
  platform preedit string itself to render live instead of staying invisible until the next commit key such as space.
- When the nested `TextEdit` is still in RichText mode, the wrapper now treats any Qt-serialized
  `<!DOCTYPE HTML ... qrichtext ...>` payload as a presentation-only document snapshot.
  It recovers visible plain text through `ContentsTextFormatRenderer.plainTextFromEditorSurfaceHtml(...)` before that
  value is reused for IME fallback, diffing, or `textEdited(...)` dispatch.
- When the user mutates the nested editor buffer locally, the wrapper now also discards any older deferred
  host-projected surface payload that was waiting behind focus/IME guards.
  A later blur or IME-settle turn therefore cannot reapply a stale RichText snapshot over the just-authored text.
- Blur-side deferred surface flush now also waits for any active IME preedit/composition session to settle first.
  Focus loss during a not-yet-space-committed Hangul word therefore gives the native input path a chance to emit the
  real committed text before any queued host surface is considered again.
- The wrapper now exposes a single cursor setter (`setCursorPositionPreservingInputMethod(...)`) that updates
  `Qt.inputMethod` together with the logical cursor move.
  Host controllers should use that wrapper-level path instead of writing `cursorPosition` into the wrapper,
  `editorItem`, and `inputItem` separately.
- When a fresh host-driven RichText surface is synced programmatically, the wrapper now also invalidates any already
  queued fallback `textEdited(...)` dispatch for the previous surface revision.
  This prevents one stale placeholder-only RichText body from being emitted after the next programmatic resource/body
  rebuild has already landed.
- Hosts can now also raise `suppressCommittedTextEditedDispatch` for one editor turn when an external file drop is
  being consumed by a higher-level resource-import path.
  While that flag is true, `TextEdit.onTextChanged` still updates native cursor/input-method bookkeeping, but the
  wrapper cancels any queued fallback `textEdited(...)` dispatch so a drop-side RichText mutation cannot be mistaken
  for an authored note-body edit.
- Hosts can now also raise `blockExternalDropMutation` while an accepted file drag/drop is hovering or being finalized.
  While that flag is true, the inner `TextEdit` becomes temporarily read-only and refuses committed-edit dispatch, so
  Qt cannot reinterpret the same file drop as editable RichText/plain-text content in parallel with the higher-level
  resource import path.
- The wrapper no longer keeps ordinary typing behind a native-`textEdited` vs deferred-fallback split.
  It now treats `TextEdit.onTextChanged` as the primary committed-edit trigger whenever:
  - the change is not inside `_programmaticTextSyncDepth`
  - no IME preedit/composition session is active
  This makes the host session track the visible editor buffer immediately instead of leaving the note-open snapshot as
  the last authoritative text until a later fallback turn arrives.
- Exposes a `textEdited(string text)` signal as a change event for the host controllers.
- The host no longer persists that whole-document RichText payload directly for ordinary typing.
- Instead, the typing controller treats the signal as a notification and derives the actual mutation from
  `getText(...)` plus the current source/plain-text bridges.
- The editor now emits the wrapper's visible plain-text snapshot through `textEdited(...)`, even when the nested
  `TextEdit.text` property currently contains Qt's serialized RichText document HTML.
  The earlier fragment-based `getFormattedText(...)` save path was removed because it leaked
  `StartFragment`/fragment-scaffold markup into note editing.
- IME composition is now treated as a first-class input state:
  - wrapper-level `inputMethodComposing` / `preeditText` surface the native `TextEdit` composition state
  - app-driven persistence/mutation dispatch is suppressed while preedit text is active
  - once the native input session settles, the wrapper forwards the resulting committed `TextEdit` state without
    replaying intermediate Hangul jamo assembly steps back into the host mutation pipeline
- Exposes direct Qt-style selection/text helpers on the wrapper itself:
  - `selectionSnapshot()`
  - `inlineFormatSelectionSnapshot()`
  - `currentPlainText()`
  - `getText(start, end)`
  - `getFormattedText(start, end)`
  - `length`
  - `hasSelection`
- The wrapper now also keeps a short-lived cache of the latest non-empty selection range.
  `inlineFormatSelectionSnapshot()` prefers the live `TextEdit` range, but if a host shortcut briefly collapses that
  range onto one selection edge during the same focus turn, the wrapper can still hand the formatting path the last
  valid selection snapshot instead of reporting an empty range.
- Host formatting controllers must consume that `inlineFormatSelectionSnapshot()` helper for RAW wrap mutations rather
  than reading `selectionSnapshot()` directly, otherwise the shortcut turn can still observe an empty selection and
  skip `<bold>` / `<italic>` / `<highlight>` tag insertion.
- That cached formatting selection is discarded as soon as focus leaves, text changes, or the caret moves away from the
  cached selection boundary, so ordinary caret-only editing does not inherit a stale format target.
- Keeps the scroll contract compatible with the existing gutter/minimap code:
  - `editorItem.parent.y` follows the `Flickable.contentItem` offset
  - `editorItem.parent.parent` resolves back to the owning `Flickable`
- Supports an external page-document scroll mode:
  - `externalScroll: true` disables the wrapper's own vertical flicking
  - `externalScrollViewport` points at the outer paper document viewport
  - `resolvedFlickable` and `contentOffsetY` let the host keep gutter/minimap math aligned to the outer paper document
- Hosts may now also supply a rendered read-only overlay through:
  - `renderedText`
  - `renderedTextFormat`
  - `showRenderedOutput`
  When enabled in plain-text input mode, the wrapper paints that rendered overlay underneath the live `TextEdit` and
  makes ordinary unselected editor glyphs transparent. This lets the app show formatted inline spans without giving
  write authority back to Qt RichText.
- That rendered overlay is automatically suppressed while an IME preedit/composition session is active.
  Hangul or other native composition text therefore remains visibly painted from the real `TextEdit` instead of being
  hidden behind one stale rendered-html snapshot.
- Preserves the legacy nested editor access pattern:
  - `editorItem`
  - `editorItem.activeFocus` (native `Item` focus state)
  - `editorItem.inputItem`
  - `editorItem.inputItem.activeFocus`
  - `editorItem.positionToRectangle(...)`
- Ordinary committed typing is now emitted directly from `TextEdit.onTextChanged`, while IME/preedit sessions still
  defer dispatch until composition settles.
- Programmatic text-sync now preserves the pre-sync logical cursor/selection range when the visible plain-text payload is
  unchanged and only the RichText markup wrapper changed (for example after inline formatting wraps).
- Selection/formatting controllers should prefer these wrapper-level Qt helpers over re-walking nested `editorItem` /
  `inputItem` objects.
- Typography is host-driven through `fontPixelSize` and `fontWeight`; the current policy is `12px` regular via
  both `ContentsDisplayView.qml` and `MobileContentsDisplayView.qml`.
- The wrapper default `fontPixelSize` itself now routes through `LV.Theme.scaleMetric(12)`, so callers that do not
  override typography still remain inside LVRS density scaling.
- The wrapper now normalizes `TextEdit.tabStopDistance` to `tabIndentSpaceCount` spaces (default: 4), measured with
  the same runtime font metrics used by the editor surface.
- The wrapper now also overrides direct `Tab` key insertion to write spaces (`tabIndentSpaceCount`, default 4) instead
  of a literal `\t`, so indent width stays consistent in both plain/rich editor modes.
- The wrapper now publishes its own `implicitHeight` from the live text-content height contract.
  Structured block hosts such as `ContentsDocumentTextBlock.qml` therefore receive a real block height instead of `0`,
  so prose blocks remain visible in the same column as inline resource/image blocks.
- The wrapper now exposes `shortcutKeyPressHandler(event)`:
  - hosts can inject a key-routing callback that runs from the inner `TextEdit.Keys.onPressed`
  - when that callback accepts the event, the wrapper stops local key fallback processing
  - this keeps host shortcuts active even while the nested `TextEdit` owns direct focus.
- That same key-routing callback is now also the place where host code can intercept plain Enter before Qt RichText
  rewrites the document on its own.
  The inline wrapper therefore remains a passive key-forwarder while RAW-authoritative line-split behavior stays owned
  by `ContentsEditorTypingController.qml`.

## Regression Notes

- This repository no longer maintains a dedicated scripted test for the page/print editor surface.
- Regression checklist:
  - RichText spans derived from `.wsnbody` tags render visibly inside the live editor surface
  - cursor/selection updates still drive gutter and minimap geometry
- programmatic note switches do not emit duplicate save mutations
- programmatic rendered-surface refresh must not loop back into host typing mutation handling as a fake user edit
- when one programmatic surface refresh supersedes another, any queued fallback `textEdited(...)` dispatch for the
  older surface must be cancelled before it reaches the typing controller
- the live note editor remains backed by `QtQuick.TextEdit`; no `LV.TextEditor` dependency should be reintroduced
- direct typing must not surface fragment comment markup such as `<!--StartFragment-->`
- any controller that needs the current visible plain-text buffer should prefer `currentPlainText()` over rebuilding the
  same full-surface snapshot ad hoc from nested `TextEdit` state
- `currentPlainText()` and `textEdited(...)` must never return Qt's serialized RichText document scaffold
  (`<!DOCTYPE HTML ... qrichtext ...>`) as if it were authored note text
- Hangul IME composition must not delete previously committed text when a syllable block is assembled
- Hangul IME composition must not leave split jamo behind after the committed syllable lands
- Hangul IME composition and desktop RichText typing must not receive a programmatic `text` reinjection while
  preedit/composition is still active
- Hangul or other IME preedit text on the desktop RichText host must remain visibly painted before the commit key is
  pressed; the word must not stay invisible until space or another explicit commit gesture lands
- Blur immediately after such an IME composition must not restore an older deferred RichText surface and erase the
  just-authored paragraph
- the host `editorSession` must not stay pinned to the note-open body snapshot after visible user typing has already
  changed the nested `TextEdit` buffer
- when `preferNativeInputHandling` is enabled, live typing/focus must not trigger whole-surface programmatic text
  reinjection before the native input session settles
- while an accepted external file drag is hovering above the editor, the nested `TextEdit` must not mutate its
  document buffer on its own; only the dedicated resource-drop path may write the resulting `<resource ... />` source
- wrapper-driven cursor restore must go through one cursor path only; the app must not fight itself by rewriting the
  same cursor position into multiple nested editor objects on the same turn
- clicking to move the cursor immediately after typing must not drop the newest word/chunk because ordinary committed
  edits are no longer held behind a wrapper-local queued dispatch timer
- desktop/mobile hosts must not feed the wrapper a fresh whole-document `renderedEditorText` payload on every
  committed keystroke; ordinary typing should reach the wrapper's programmatic sync path only after the presentation
  debounce or an explicit flush
- iOS cursor movement and selection gestures from the software keyboard must still work after wrapper-level cursor,
  selection, or scroll changes; `Qt.inputMethod.update(...)` coverage must not regress
- iOS keyboard-driven range selection must not collapse to only the newest delta after the wrapper reapplies text or
  selection state; the original anchor edge must survive the sync path
- iOS touch double-tap must keep selecting the touched word even though the editor is mounted inside a `Flickable`
- iOS touch triple-tap must expand to the surrounding paragraph instead of leaving only the insertion cursor behind
- when native-input mode is active and the software keyboard is hidden, dragging on the editor surface must scroll
  without triggering immediate cursor move/focus on press
- in that same hidden-keyboard state, a single tap must place the cursor, focus the editor, and show the keyboard once
- the visible editor text size must follow the host-supplied platform policy instead of falling back to the legacy
  `12px` default
  - the visible desktop editor text weight must follow the host-supplied regular-weight policy instead of staying at a
    heavier medium default
  - page/print hosts must be able to switch the editor into the outer paper document scroll path without re-enabling
    nested internal flicking
- pressing `Tab` in the note editor must render one indent step as approximately four spaces, not an oversized default
  tab column
- pressing `Tab` must insert four literal spaces by default (not `\t`) so RAW/renderer mode switches do not inflate
  indentation width
- mixed text + `<resource ... />` note bodies must keep text blocks visible before and after inline image blocks; a
  structured text block host must never collapse to `0` height just because the inline editor wrapper omitted its own
  `implicitHeight`
- the visible caret must remain painted whenever the nested `TextEdit` still owns keyboard focus, even if the wrapper
  FocusScope itself is not the current `activeFocus` item
- injected host shortcut handlers must still fire when the nested `TextEdit` has focus, and accepted shortcut events
  must not fall through into literal character insertion.
- Immediately after a host shortcut such as `Cmd/Ctrl+B`, `Cmd/Ctrl+I`, or highlight formatting, the wrapper must not
  report an empty selection just because the live `TextEdit` momentarily collapsed the range to one edge while the
  RAW formatting mutation is reading the current selection on that same turn.
- A plain-text structured paragraph that contains RAW inline style tags must still paint visible formatted spans once
  the host provides `renderedText`; the wrapper must not regress to a plain unformatted view just because the input
  engine itself stayed in `TextEdit.PlainText`.
- While that formatted overlay is active, starting an IME composition must immediately reveal the native composing text
  instead of leaving the editor visually frozen on the previous rendered-html snapshot.
