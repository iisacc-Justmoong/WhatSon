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
  - double-tap reselects the touched word via `TextEdit.selectWord()`
  - triple-tap expands to the surrounding newline-delimited paragraph
  - the resulting programmatic selection still reuses the same input-method update path so iOS selection UI stays in
    sync
- Hosts can opt into `preferNativeInputHandling`:
  - while the editor keeps focus or an IME preedit session is active, app-driven `text` resync is deferred
  - the latest deferred payload is flushed after focus leaves or when no native input session is active anymore
  - this lets mobile keep OS-native composition, double-tap selection, and repeat-backspace behavior ahead of the
    app's RichText surface normalization
- Exposes a `textEdited(string text)` signal as a change event for the host controllers.
- The host no longer persists that whole-document RichText payload directly for ordinary typing.
- Instead, the typing controller treats the signal as a notification and derives the actual mutation from
  `getText(...)` plus the current source/plain-text bridges.
- The editor still emits the live `TextEdit.text` payload for formatting-oriented consumers. The earlier
  fragment-based `getFormattedText(...)` save path was removed because it leaked `StartFragment`/fragment-scaffold
  markup into note editing.
- IME composition is now treated as a first-class input state:
  - wrapper-level `inputMethodComposing` / `preeditText` surface the native `TextEdit` composition state
  - `textEdited(...)` is deferred while preedit text is active
  - the wrapper emits one committed edit after composition ends instead of replaying every intermediate Hangul syllable
    assembly step back into the host mutation pipeline
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
- when `preferNativeInputHandling` is enabled, live typing/focus must not trigger whole-surface programmatic text
  reinjection before the native input session settles
- desktop/mobile hosts must not feed the wrapper a fresh whole-document `renderedEditorText` payload on every
  committed keystroke; ordinary typing should reach the wrapper's programmatic sync path only after the presentation
  debounce or an explicit flush
- iOS cursor movement and selection gestures from the software keyboard must still work after wrapper-level cursor,
  selection, or scroll changes; `Qt.inputMethod.update(...)` coverage must not regress
- iOS keyboard-driven range selection must not collapse to only the newest delta after the wrapper reapplies text or
  selection state; the original anchor edge must survive the sync path
- iOS touch double-tap must keep selecting the touched word even though the editor is mounted inside a `Flickable`
- iOS touch triple-tap must expand to the surrounding paragraph instead of leaving only the insertion cursor behind
- the visible editor text size must follow the host-supplied platform policy instead of falling back to the legacy
  `12px` default
  - the visible desktop editor text weight must follow the host-supplied regular-weight policy instead of staying at a
    heavier medium default
  - page/print hosts must be able to switch the editor into the outer paper document scroll path without re-enabling
    nested internal flicking
