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
- The wrapper now exposes a single cursor setter (`setCursorPositionPreservingInputMethod(...)`) that updates
  `Qt.inputMethod` together with the logical cursor move.
  Host controllers should use that wrapper-level path instead of writing `cursorPosition` into the wrapper,
  `editorItem`, and `inputItem` separately.
- The wrapper no longer keeps a separate `0ms` queued committed-edit timer for ordinary typing.
  Ordinary edits now prefer the native `TextEdit` edited signal when that path exists, and older runtimes only fall
  back to one deferred `textChanged` re-check after the platform input session settles.
  The wrapper no longer keeps its own synthetic `_compositionEditPending` commit state.
- Exposes a `textEdited(string text)` signal as a change event for the host controllers.
- The host no longer persists that whole-document RichText payload directly for ordinary typing.
- Instead, the typing controller treats the signal as a notification and derives the actual mutation from
  `getText(...)` plus the current source/plain-text bridges.
- The editor still emits the live `TextEdit.text` payload for formatting-oriented consumers. The earlier
  fragment-based `getFormattedText(...)` save path was removed because it leaked `StartFragment`/fragment-scaffold
  markup into note editing.
- IME composition is now treated as a first-class input state:
  - wrapper-level `inputMethodComposing` / `preeditText` surface the native `TextEdit` composition state
  - app-driven persistence/mutation dispatch is suppressed while preedit text is active
  - once the native input session settles, the wrapper forwards the resulting committed `TextEdit` state without
    replaying intermediate Hangul jamo assembly steps back into the host mutation pipeline
- Exposes direct Qt-style selection/text helpers on the wrapper itself:
  - `selectionSnapshot()`
  - `getText(start, end)`
  - `getFormattedText(start, end)`
  - `length`
  - `hasSelection`
- Keeps the scroll contract compatible with the existing gutter/minimap code:
  - `editorItem.parent.y` follows the `Flickable.contentItem` offset
  - `editorItem.parent.parent` resolves back to the owning `Flickable`
- Supports an external page-document scroll mode:
  - `externalScroll: true` disables the wrapper's own vertical flicking
  - `externalScrollViewport` points at the outer paper document viewport
  - `resolvedFlickable` and `contentOffsetY` let the host keep gutter/minimap math aligned to the outer paper document
- Preserves the legacy nested editor access pattern:
  - `editorItem`
  - `editorItem.activeFocus` (native `Item` focus state)
  - `editorItem.inputItem`
  - `editorItem.inputItem.activeFocus`
  - `editorItem.positionToRectangle(...)`
- Falls back to `TextEdit.onTextChanged` dispatch when a native `textEdited` signal is unavailable, while suppressing
  programmatic text-sync loops.
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
- The wrapper now exposes `shortcutKeyPressHandler(event)`:
  - hosts can inject a key-routing callback that runs from the inner `TextEdit.Keys.onPressed`
  - when that callback accepts the event, the wrapper stops local key fallback processing
  - this keeps host shortcuts active even while the nested `TextEdit` owns direct focus.

## Regression Notes

- This repository no longer maintains a dedicated scripted test for the page/print editor surface.
- Regression checklist:
  - RichText spans derived from `.wsnbody` tags render visibly inside the live editor surface
  - cursor/selection updates still drive gutter and minimap geometry
  - programmatic note switches do not emit duplicate save mutations
  - the live note editor remains backed by `QtQuick.TextEdit`; no `LV.TextEditor` dependency should be reintroduced
- direct typing must not surface fragment comment markup such as `<!--StartFragment-->`
- Hangul IME composition must not delete previously committed text when a syllable block is assembled
- Hangul IME composition must not leave split jamo behind after the committed syllable lands
- Hangul IME composition and desktop RichText typing must not receive a programmatic `text` reinjection while
  preedit/composition is still active
- The wrapper must prefer the native `TextEdit` edited-signal path over app-synthesized composition commit state when
  the runtime provides that signal.
- when `preferNativeInputHandling` is enabled, live typing/focus must not trigger whole-surface programmatic text
  reinjection before the native input session settles
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
- injected host shortcut handlers must still fire when the nested `TextEdit` has focus, and accepted shortcut events
  must not fall through into literal character insertion.
