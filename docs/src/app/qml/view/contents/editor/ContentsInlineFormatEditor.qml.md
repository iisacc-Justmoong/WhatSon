# `src/app/qml/view/contents/editor/ContentsInlineFormatEditor.qml`

## Responsibility

Wraps the live `LV.TextEditor` used by the note document surface.

## Current Contract

- The editable surface is always `TextEdit.PlainText` through `LV.TextEditor`. In rendered mode that native surface
  contains the visible logical projection, while the component `text` property remains the RAW `.wsnbody` source.
- `renderedText` is an optional read-only `TextEdit.RichText` overlay derived from the renderer pipeline.
- The RichText overlay is disabled for focus and key input. It may paint formatted text and resource frame images, but
  ordinary editing remains routed to the underlying `LV.TextEditor`.
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
  inline tag bytes. Projected cursor position is never used as the source of truth for deletion.
- Cursor visibility is explicit and mutually exclusive. The native cursor delegate is visible only for the plain source
  surface, while the component paints a projected cursor above the RichText overlay only for collapsed caret positions.
  Non-empty selection ranges hide the projected cursor so the selection model owns the interaction state.
- Rendered-surface mouse clicks update a local projected-cursor logical offset immediately after restoring the mapped
  source cursor, so the visible caret moves under the pointer even before the parent host republishes
  `logicalCursorPosition`.
- The RichText overlay remains visible while `LV.TextEditor` has a non-empty native selection. The component mirrors
  the source range into rendered coordinates instead of exposing RAW tag geometry.
- Native selection paint stays enabled for visible logical content even while the rendered overlay is visible. The
  underlying logical glyph paint is transparent, so OS/Qt drag and Shift selection remain visible without showing a
  second text layer.
- Source ranges that contain only hidden formatting tag tokens, including empty `<highlight></highlight>` or nested
  empty format wrappers, do not paint native or rendered selection because they do not represent visible editor
  content.
- Renderer-owned `normalizedHtmlBlocks` are used only to identify iiHtmlBlock resource display blocks for selection
  presentation. When a selection intersects a resource source span, the resource contributes one atomic selection
  rectangle; the RAW `<resource ... />` string must never paint as multiple selected text runs.
- While the rendered overlay is hidden, the rendered surface does not add pointer handlers above the `LV.TextEditor`;
  OS/Qt pointer selection remains the selection gesture path.
- While the rendered overlay is visible, RAW offsets and RichText document positions are not used as native surface
  character positions. Pointer interaction is resolved against `contentsInlineFormatRenderedGeometryProbe`, a
  transparent plain-text `TextEdit` whose text is exactly `displayGeometryText`. The resulting visible logical
  cursor/selection range is mapped back through the supplied C++ `coordinateMapper` and restored on the native logical
  surface, while the public selection/cursor properties expose the authoritative RAW offsets. This makes dragging
  select visible character ranges instead of dragging the cursor across hidden tag bytes or RichText HTML positions.
  The transparent surface-selection `TextEdit` mirrors the same plain logical text for selection-surface
  synchronization. The local pointer cursor override is applied only when that restored range is collapsed; non-empty
  drag selections clear the override. The same bridge preserves native-style multi-click granularity: the second click
  in a pointer sequence selects the visible logical line, and the third click selects the visible paragraph before
  mapping that range back to RAW source offsets. This bridge is disabled while native IME composition is active.
- Rendered pointer hit testing clamps line-adjacent mapped coordinates into the rendered logical text area's measured
  content bounds before calling `positionAt(...)`. Clicks and drags that land in the vertical slack around a line
  therefore resolve to that visible line, while clicks far below the rendered body still resolve to the terminal body
  position.
- The editor body is top-flush: `LV.TextEditor` vertical inset and rendered-overlay padding stay at zero.
- Keeps a disabled, transparent plain `TextEdit` geometry probe in sync with the logical display text for cursor hit
  testing, pointer selection, projected caret placement, and source-to-rendered selection bounds.
- Reports `displayContentHeight` from the actual RichText overlay while rendered output is visible so the scrollable
  editor viewport follows the rendered body height.
- Mounts `ContentsEditorGeometryProvider` as the only view-owned geometry adapter for chrome measurements. The adapter
  receives TextEdit/resource items, measures visible line rows and logical-line row rectangles, then publishes value
  snapshots.
- Mounts `ContentsEditorVisualLineMetrics` as the C++ owner of minimap row normalization. It receives measured visual
  line count and row-width ratios only; it never receives TextEdit, cursor, selection, or resource overlay objects.
- Mounts `ContentsLineNumberRailMetrics` as the C++ owner of logical line-number row construction. The inline editor
  binds source/projection inputs and measured geometry rows into the metrics object, then exposes `logicalGutterRows`
  from its resolved `rows`. The rows count logical lines, not visual wrap rows; a wrapped paragraph contributes one row
  whose height covers all wrapped visual rows, while resource frames contribute one row with the rendered frame height.
  Row y values are produced by C++ and validated there so later line numbers cannot collapse onto the first line's y
  position. The metrics object never receives TextEdit, cursor, selection, or resource overlay objects directly.
- When iiHtmlBlock resource spans are present, the rendered overlay is pinned even during composition so ordinary
  typing cannot expose the RAW `<resource ... />` tag while the renderer catches up.
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
range back to authoritative `.wsnbody` offsets. The rendered overlay receives a synchronized logical selection range,
and iiHtmlBlock resource spans are painted as one block-level selection rectangle. During active IME composition,
programmatic text sync remains deferred through the native editor path. For resource-backed projections the overlay
stays pinned above the plain logical surface, so a transient parser/render turn cannot collapse a framed image back to
the RAW resource tag. Mouse pointer selection follows the same logical-to-source table: pointer coordinates are
measured against the plain `displayGeometryText` probe and the pointer bridge restores the matching visible
cursor/selection span on the native logical surface while exposing RAW offsets for formatting commands and persistence.
The surface-selection editor mirrors the same plain logical text. A local pointer cursor override keeps the projected caret at
the clicked logical offset only for collapsed pointer clicks until the host catches up or the next non-pointer cursor
movement clears it. Once the pointer gesture expands into a non-empty selection, the override is cleared and the
rendered/native selection surfaces own the visual state. Double-click and triple-click gestures follow the same
logical-to-source mapping, selecting the visible line and paragraph respectively instead of exposing hidden RAW tag
bytes. During IME composition that pointer bridge is inactive and the native editor receives the pointer path directly.
Collapsed RAW selections that contain only hidden opening/closing inline formatting wrappers are kept collapsed after
projection to the native logical surface, so restoring a hidden tag range cannot select the next visible character.
Rendered-mode Backspace, Delete, and typing all flow through the visible text mutation payload, which rewinds/advances
around hidden inline boundary tags before mutating RAW source.
