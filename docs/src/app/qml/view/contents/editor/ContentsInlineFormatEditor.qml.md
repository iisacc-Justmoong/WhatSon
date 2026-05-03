# `src/app/qml/view/contents/editor/ContentsInlineFormatEditor.qml`

## Responsibility

Wraps the live `LV.TextEditor` used by the note document surface.

## Current Contract

- The editable buffer is always `TextEdit.PlainText` through `LV.TextEditor`.
- `renderedText` is an optional read-only `TextEdit.RichText` overlay derived from the renderer pipeline.
  The overlay intentionally uses Qt's text-edit renderer, not `Text`, so the visible rendered layer and the native
  plain `LV.TextEditor` fallback used during IME composition share the same text paint path.
- The RichText overlay is disabled for input. It may paint formatted text and resource frame images, but it must not
  accept focus, pointer, selection, or key events; ordinary editing remains routed to the underlying `LV.TextEditor`.
- Cursor visibility is explicit and mutually exclusive. The native `LV.TextEditor` receives a colored cursor delegate
  only for the fallback/plain source surface, while the component paints a projected cursor above the RichText overlay
  from the same visible-display geometry probe whenever rendered output is visible. This keeps the WYSIWYG caret
  visible and blinking without also painting a second RAW-surface caret under rendered output.
- The RichText overlay remains visible while `LV.TextEditor` has a non-empty native selection. The native editor still
  owns the source selection range, but its RAW selection paint stays transparent while rendered output is visible. The
  component mirrors that source range into the rendered overlay so visible prose is selected in rendered coordinates
  instead of exposing RAW tag geometry.
- Renderer-owned `normalizedHtmlBlocks` are used only to identify iiHtmlBlock resource display blocks for selection
  presentation. When a selection intersects a resource source span, the resource contributes one atomic selection
  rectangle; the RAW `<resource ... />` string must never paint as multiple selected text runs.
- A rendered-surface tap handler is active only while rendered output is visible. It asks the visible logical
  text geometry for the tapped position, maps that logical offset through `logicalToSourceOffsets`, and then moves the
  RAW `LV.TextEditor` cursor to the matching source offset. The handler does not replace RAW persistence; it only makes
  cursor placement follow the WYSIWYG surface instead of hidden tag text.
- The editor body is top-flush: `LV.TextEditor` vertical inset and rendered-overlay padding stay at zero. Horizontal
  text-column spacing is preserved with item margins instead of internal top padding.
- Keeps a disabled, transparent plain `TextEdit` geometry probe in sync with the logical display text for cursor hit
  testing and projected caret placement.
- Exposes line-start geometry from the actual RichText overlay while rendered output is visible. Gutter line numbers
  therefore see resource image frames and other rendered block heights instead of the shorter plain-text probe.
- Reports `displayContentHeight` from the actual RichText overlay while rendered output is visible so the scrollable
  editor viewport follows the rendered body height.
- When the rendered overlay is hidden for native composition, geometry falls back to the live `LV.TextEditor.editorItem`
  and uses the supplied source position. The hidden-overlay fallback still uses the same Qt text-edit rendering family
  as the overlay, so typing during composition and the committed render after a space or punctuation terminator do not
  switch between unrelated text renderers.
- `textEdited(text)` reports plain RAW text upward.
- `tagManagementKeyPressHandler` is the only key hook and is limited to explicit tag-management shortcuts.
  Modifier-only key presses are ignored so the LVRS wrapper does not turn bare Control/Option/Meta transitions into
  RAW tag commands.
- `Ctrl+Alt+C` / `Meta+Alt+C` are mirrored through a focused-editor `Shortcut` because some `LV.TextEditor`
  runtime builds consume that chord inside the native text item before parent `Keys` handlers can see it.
- `ContentsInlineFormatEditorController` is mounted as the C++/QML helper bridge for focus, selection snapshots, native
  composition checks, and programmatic text-sync policy against `LV.TextEditor.editorItem`.
- `restoreSelectionRange(...)` restores non-empty ranges through native `moveCursorSelection(...)` so Qt keeps the
  selection active instead of collapsing it to a cursor-only position.
- `focusTerminalBodyPosition()` focuses the native `LV.TextEditor`, clears any stale selection, and moves the cursor to
  the RAW text end through `setCursorPositionPreservingNativeInput(...)`. This is used only by the parent structured
  flow's bottom-empty-area hit target.
- Exposes `positionToRectangle(position, sourcePosition)` and `mapEditorPointToItem(...)` as narrow geometry hooks for
  the gutter sampler. `positionToRectangle(...)` returns visible line-start geometry, while `mapEditorPointToItem(...)`
  converts that point into the gutter coordinate space.
- Ordinary navigation, selection, Backspace/Delete repeat, paste fallback, and IME/preedit behavior remain with Qt's
  native text-editing path exposed by `LV.TextEditor`.

## Pipeline Position

The component displays `editorSurfaceHtml` generated by `ContentsEditorPresentationProjection`, but it never serializes
RichText back into `.wsnbody`. The authored source remains the plain `LV.TextEditor` buffer. During native text
selection, the rendered overlay remains visible while the underlying plain editor keeps ownership of the source range
and paints no RAW highlight. The rendered overlay receives a synchronized logical selection range, and iiHtmlBlock
resource spans are painted as one block-level selection rectangle. During active IME composition, the rendered overlay
is temporarily hidden and the plain editor text is restored to the foreground so preedit behavior stays native without
switching between unrelated text renderer families. While the rendered overlay is visible, `logicalCursorPosition` maps
the RAW cursor to the logical display text so the projected caret follows the visible text rather than hidden markup;
surface clicks perform the inverse logical-to-RAW offset conversion before moving the underlying editor cursor. The
native cursor delegate is hidden in that state and re-enabled only when the rendered overlay is unavailable.
