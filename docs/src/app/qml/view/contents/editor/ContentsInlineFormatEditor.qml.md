# `src/app/qml/view/contents/editor/ContentsInlineFormatEditor.qml`

## Responsibility

Wraps the live `LV.TextEditor` used by the note document surface.

## Current Contract

- The editable surface is always `TextEdit.PlainText` through `LV.TextEditor`. In rendered mode that native surface
  contains the visible logical projection, while the component `text` property remains the RAW `.wsnbody` source.
- `renderedText` is an optional read-only `TextEdit.RichText` overlay derived from the renderer pipeline.
- The RichText overlay is disabled for focus and key input. It paints formatted text only; resource frames are painted
  by a structured visual-block layer. Ordinary editing remains routed to the underlying `LV.TextEditor`.
- The RichText overlay is stacked below the transparent native editor surface. This preserves WYSIWYG paint while
  keeping keyboard selection, Shift selection, IME composition, and plain-source pointer selection on `LV.TextEditor`.
- The mounted `LV.TextEditor` opts into native gesture handling so the LVRS hover/focus mouse surface does not sit
  between the OS pointer stream and Qt's text selection machinery.
- Public programmatic text replacement APIs delegate to `ContentsInlineFormatEditorController`, so focused native
  selection and composition policy can reject or defer host-side surface refresh instead of clearing OS selection.
- The native `LV.TextEditor.text` property is not bound directly to the projection expression. Initial load and later
  host-side projection refreshes must call the inline editor controller's programmatic sync path, so every replacement
  runs through the same focused-input defer/reject policy.
- WYSIWYG mapping policy delegates to C++ `ContentsWysiwygEditorPolicy`. QML keeps the `TextEdit` geometry and pointer
  surface, but source-tag parsing, visible-text mutation planning, hidden-tag cursor normalization, visible logical
  selection mapping, line/paragraph range decisions, and atomic resource-block selection decisions are not implemented
  in QML.
- Explicit tag-management mutations use `applyTagManagementMutationPayload(...)`, which bypasses host-side sync
  deferral because the command is initiated by the focused editor itself, restores the returned source selection, and
  emits the normal `textEdited(...)` signal for RAW persistence.
- Native text edits in rendered mode are diffed against the visible logical projection and converted into a RAW
  `.wsnbody` splice by `ContentsWysiwygEditorPolicy.visibleTextMutationPayload(...)`. Backspace therefore stays on the
  OS/Qt text-editing path, but the committed source mutation deletes the previous visible glyph instead of hidden
  inline tag bytes. A separate projected cursor position is not kept, so deletion always follows the native edit cursor.
- Cursor visibility has one owner. The `LV.TextEditor` cursor delegate stays visible in both plain-source and rendered
  overlay modes when the editor is focused and the selection is collapsed. No separate projected cursor `Rectangle` or
  pointer cursor override is mounted.
- The RichText overlay remains visible while `LV.TextEditor` has a non-empty native selection. It does not own or paint
  a second selection; the visible selection highlight is the same native `LV.TextEditor` selection that receives edit
  commands.
- Native selection paint stays enabled for visible logical content even while the rendered overlay is visible. The
  underlying logical glyph paint is transparent, so OS/Qt drag and Shift selection remain visible without showing a
  second text layer.
- Source ranges that contain only hidden formatting tag tokens, including empty `<highlight></highlight>` or nested
  empty format wrappers, do not paint native selection because they do not represent visible editor content.
- Parser-owned `documentBlocks` are used first for atomic resource selection and visual-block geometry. Renderer-owned
  `normalizedHtmlBlocks` remain compatibility metadata for HTML projection spans, not a separate selection layer.
- The visible RichText overlay receives an already flow-corrected `renderedText` from the C++ display backend.
  Resource frames are still painted by direct visual delegates, while the backend-provided transparent spacer keeps
  ordinary text after a frame in the same y-coordinate flow as the gutter and minimap geometry.
- The rendered overlay visibility follows rendered projection availability, not native composition state. During typing
  and IME composition the RichText projection stays mounted so previously formatted lines do not collapse to the
  plain logical editor surface.
- While the rendered overlay is hidden, the rendered surface does not add pointer handlers above the `LV.TextEditor`;
  OS/Qt pointer selection remains the selection gesture path.
- While the rendered overlay is visible, RAW offsets and RichText document positions are not used as native surface
  character positions. Pointer interaction is resolved against `contentsInlineFormatRenderedGeometryProbe`, a
  transparent plain-text `TextEdit` whose text is exactly `displayGeometryText`. The resulting visible logical
  cursor/selection range is mapped back through the supplied C++ `coordinateMapper` and restored on the native logical
  surface, while the public selection/cursor properties expose the authoritative RAW offsets. This makes dragging
  select visible character ranges instead of dragging the cursor across hidden tag bytes or RichText HTML positions.
  The transparent surface-selection `TextEdit` is a passive mirror of the same plain logical text and does not accept
  focus, mouse selection, or key events. Pointer gestures store the logical start/end/cursor span in explicit QML
  properties before mapping that span back to RAW source offsets and restoring the live `LV.TextEditor` selection. The
  same bridge preserves native-style multi-click granularity: the second click in a pointer sequence selects the
  visible logical line, and the third click selects the visible paragraph before mapping that range back to RAW source
  offsets. This bridge is disabled while native IME composition is active.
- Rendered pointer hit testing clamps line-adjacent mapped coordinates into the rendered logical text area's measured
  content bounds before calling `positionAt(...)`. Clicks and drags that land in the vertical slack around a line
  therefore resolve to that visible line, while clicks far below the rendered body still resolve to the terminal body
  position.
- The editor body is top-flush while retaining the legacy bottom breathing room: `LV.TextEditor.editorItem.topPadding`
  and rendered-overlay top padding stay at zero, while `displayContentHeight` explicitly adds `LV.Theme.gap16` after
  the measured body text height.
- Keeps a disabled, transparent plain `TextEdit` geometry probe in sync with the logical display text for pointer hit
  testing and visible-logical selection mapping.
- Reports `displayTextContentHeight` from the RichText overlay height while rendered output is visible. That overlay
  already contains backend-generated resource flow spacers, so `renderedOverlay.contentHeight` is the visual body flow
  height. `displayContentHeight` adds the restored bottom inset, while `displayBodyHeight` stays equal to the measured
  body so terminal hit testing does not treat the bottom breathing room as rendered text.
- Mounts `ContentsEditorGeometryProvider` as the only view-owned geometry adapter for chrome measurements. The adapter
  receives the logical TextEdit geometry item, explicit structured resource visual blocks, measures visible line rows
  and logical-line row rectangles, then publishes value snapshots. Line-number text rows are measured against the plain
  logical display probe. Resource rows contribute an explicit visual-block height delta to later rows, but ordinary
  gutter rows keep their own logical text y snapshots instead of asking the RichText overlay or rendered HTML string to
  map logical offsets after resource blocks.
- Mounts `ContentsEditorVisualLineMetrics` as the C++ owner of minimap row normalization. It receives measured visual
  line count and row-width ratios only; it never receives TextEdit, cursor, selection, or resource overlay objects.
- Mounts `ContentsLineNumberRailMetrics` as the C++ owner of logical line-number row construction. The inline editor
  binds source/projection inputs and measured geometry rows into the metrics object, then exposes `logicalGutterRows`
  from its resolved `rows`. The rows count logical lines, not visual wrap rows; a wrapped paragraph contributes one row
  whose height covers all wrapped visual rows, while resource frames contribute one row with one gutter-line height.
  Row y values are produced by C++ and validated there so later line numbers cannot collapse onto the first line's y
  position. The metrics object never receives TextEdit, cursor, selection, or resource overlay objects directly.
- Exposes the raw `lineNumberGeometryRows` and `lineNumberGeometryResourceBlockHeights` snapshots as read-only
  diagnostics for the gutter pipeline. The painted rail still consumes `logicalGutterRows`.
- Pointer selection checks measured structured resource visual rows before falling back to plain-text `positionAt(...)`.
  A hit inside an image/resource frame selects the atomic resource boundary for that block, so the frame cannot behave
  like it contains selectable virtual text lines.
- Pointer selection below a structured resource frame subtracts the resource visual-height delta before asking the
  plain geometry probe for a logical text offset. This keeps clicks on prose below an image aligned with the text that
  was shifted down by the transparent overlay spacer.
- Resource-frame gutter anchoring assumes the resource block contributes the generated frame image height reported by
  `resourceVisualBlocks`. The resource presentation controller emits visual block records for QML delegates; the
  geometry path no longer parses rendered HTML to find the frame height. The next gutter row is expected to land at
  `resourceRow.y + frameHeight`.
- If the plain geometry probe reports the first text row after a resource frame and the following blank logical row at
  the same y coordinate, the C++ geometry adapter separates those rows by their published line height before the rail
  paints them.
- Collapsed cursor placement also treats the resource boundary as a non-text atomic block. If native cursor movement
  lands on the resource line, the C++ WYSIWYG policy moves it to the nearest prose boundary outside the
  frame, trying the opposite boundary when the preferred side is still inside the resource row; when no outside
  boundary exists, the native caret is hidden rather than painted inside the image frame.
- The rendered overlay stays pinned during composition and ordinary native typing so active edits cannot expose plain
  logical text or RAW-shaped resource placeholders while the renderer catches up.
- `textEdited(text)` reports plain RAW text upward.
- `tagManagementKeyPressHandler` is the only key hook and is limited to explicit tag-management shortcuts.
- The explicit body-tag shortcut surface forwards `Ctrl/Meta+Alt+C` for callout and `Ctrl/Meta+Alt+A` for agenda to
  the same tag-management hook used by formatting commands.
- The explicit highlight formatting shortcut is `Cmd/Ctrl+Shift+E`; legacy `H` shortcuts are not part of this surface.
- `ContentsInlineFormatEditorController` is mounted as the C++ input controller for focus, selection snapshots, native
  composition checks, local selection interaction tracking, and programmatic text-sync policy against
  `LV.TextEditor.editorItem`; no QML helper owns that controller state.
- User-initiated tag-management commands apply through the controller's immediate programmatic text path before
  restoring the source selection returned by the tag insertion controller.
- `restoreSelectionRange(...)` focuses the live `LV.TextEditor`, clears stale collapsed selections before cursor
  placement, and restores non-empty ranges through native `moveCursorSelection(...)`.
- `focusTerminalBodyPosition()` focuses the native `LV.TextEditor`, clears any stale selection, and moves the cursor to
  the visible terminal position; `sourceCursorPosition` maps that collapsed caret back to the RAW text end.
- Ordinary navigation, selection, Backspace/Delete repeat, paste fallback, and IME/preedit behavior remain with Qt's
  native text-editing path exposed by `LV.TextEditor`. In rendered mode, the resulting visible logical text delta is
  translated to a RAW character range before source mutation.
- Rendered-overlay mouse drag selection is translated from logical visible text coordinates back to RAW source offsets.
  Collapsed clicks follow the same path and are treated as cursor placement, not as a no-op selection gesture.
  Plain-source selection, keyboard selection, Shift-based selection, and IME/preedit behavior remain owned by
  `LV.TextEditor`; the rendered-overlay pointer bridge is inactive during IME composition.
- Rendered-mode native cursor movement happens in visible logical coordinates. `sourceCursorPosition`,
  `sourceSelectionStart`, and `sourceSelectionEnd` map those positions back to RAW offsets and trim hidden-only inline
  wrapper selections so empty formatting tags cannot select the following visible character.
- The tag-management key filter must set unhandled key events back to `accepted = false`; otherwise ordinary
  navigation and Shift-selection are swallowed before the native editor sees them.

## Pipeline Position

The component displays `editorSurfaceHtml` generated by `ContentsEditorPresentationProjection`, but it never serializes
RichText back into `.wsnbody`. The authored source remains the component `text` property and every edit emits RAW text
through `textEdited(...)`. During rendered-mode native editing, the `LV.TextEditor` buffer is the visible logical text
projection; `visibleTextMutationPayload(...)` converts the committed text delta back to a source splice before the
parent persistence path sees it. Host-side projection refresh is not a live `LV.TextEditor.text` binding; it is an
explicit controller-mediated sync so focused native edits, selections, and IME composition cannot be overwritten by a
transient RAW source snapshot. During native text selection, the rendered overlay remains visible while the underlying
plain editor keeps ownership of the visible logical range; the exported `sourceSelectionStart/End` values map that
range back to authoritative `.wsnbody` offsets. The rendered overlay selection remains cleared, and no iiHtmlBlock
resource selection rectangle is painted separately. During active IME composition,
programmatic text sync remains deferred through the native editor path, but the RichText overlay remains visible.
For resource-backed and styled text projections the overlay stays pinned above the plain logical surface, so a
transient parser/render turn cannot collapse formatted text or a framed image back to plain text/RAW placeholders.
Mouse pointer selection follows the same logical-to-source table: pointer coordinates are
measured against the plain `displayGeometryText` probe and the pointer bridge restores the matching visible
cursor/selection span on the native logical surface while exposing RAW offsets for formatting commands and persistence.
The surface-selection editor only mirrors that plain logical text; it is not focusable and never owns the edit cursor.
The explicit selection snapshot exposes both logical and source fields, so RAW selection/cursor offsets are not hidden
behind the generic `cursorPosition` name. The visible caret is the same native `LV.TextEditor` cursor delegate that
receives typing and Backspace. Once the pointer gesture expands into a non-empty selection, the native selection model
owns the visual state. Double-click and triple-click gestures follow the same
logical-to-source mapping, selecting the visible line and paragraph respectively instead of exposing hidden RAW tag
bytes. During IME composition that pointer bridge is inactive and the native editor receives the pointer path directly.
Collapsed RAW selections that contain only hidden opening/closing inline formatting wrappers are kept collapsed after
projection to the native logical surface, so restoring a hidden tag range cannot select the next visible character.
Rendered-mode Backspace, Delete, and typing all flow through the visible text mutation payload, which rewinds/advances
around hidden inline boundary tags before mutating RAW source.
