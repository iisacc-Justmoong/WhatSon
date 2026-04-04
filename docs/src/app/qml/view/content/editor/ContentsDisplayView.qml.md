# `src/app/qml/view/content/editor/ContentsDisplayView.qml`

## Responsibility

`ContentsDisplayView.qml` is the shared editor surface for desktop and mobile note editing. It composes the custom
inline-format editor, gutter, minimap, and lower drawer into one geometry contract so every collaborator can resolve line
positions from the same document origin.

The shared surface also owns the cross-platform editor typography policy:

- desktop editor text renders at `13px`
- mobile editor text renders at `15px` (`desktop + 2`)
- mobile hides the gutter entirely instead of reserving a transparent gutter column

## Composition Model

`ContentsDisplayView.qml` now delegates selection caching, inline-format mutation, and formatting context-menu dispatch
to `ContentsEditorSelectionController.qml`.

The root file keeps wrapper functions such as `selectedEditorRange()`, `wrapSelectedEditorTextWithTag(...)`, and
`handleSelectionContextMenuEvent(...)` so the wider editor shell preserves a stable surface while selection-specific
logic lives in a separate sibling controller.

The root editor state now keeps two text projections:
- `editorText`: canonical `.wsnbody` source tags (`<bold>`, `<italic>`, ...)
- `renderedEditorText`: RichText rendering projection produced just before binding into
  `ContentsInlineFormatEditor`

## Geometry Contract

- `editorDocumentStartY` is the canonical top inset for the editable document.
- The visible `16px` top spacer is materialized through the outer editor surface top margin, not through a late
  mutation of `editorItem.y`.
- `editorDocumentTopPadding` remains `0` so LVRS vertical centering does not reintroduce hidden top padding inside the
  text area.
- `editorSurfaceHeight` is derived from `editorViewportHeight - editorDocumentStartY`, which keeps Fill sizing stable
  after reserving the fixed top spacer.
- `effectiveEditorFontPixelSize` is the canonical editor font size and now drives both the editable RichText surface
  and the legacy formatted-preview fallback.
- `editorTextLineBoxHeight` follows that same effective font size so gutter/minimap fallback geometry does not keep the
  old `12px` line-box assumption after the editor typography changed.
- `logicalTextLength()` is the canonical line-metric length source for gutter/minimap cursor geometry in RichText mode.
  It is derived from `ContentsLogicalTextBridge` logical offsets/counts instead of raw `editorText.length`, so markup
  token length cannot desynchronize gutter row placement.
- `logicalLineDocumentYCache` stores monotonic document-Y values per logical line. It is rebuilt per
  `gutterRefreshRevision` and reused by both `lineDocumentY(...)` and
  `logicalLineNumberForDocumentY(...)` so binary-search visibility and rendered gutter delegates never drift into
  overlapping or skipped line-number rows when empty lines/wrapped edits jitter `positionToRectangle(...)`.
- `visibleGutterLineEntries` is an imperative snapshot (`[{ lineNumber, y }]`) refreshed from
  `commitGutterRefresh()`. The gutter no longer asks the live editor geometry for every delegate binding evaluation.
- Minimap viewport/current-line chrome is also exposed as numeric snapshots
  (`minimapResolvedViewportHeight`, `minimapResolvedViewportY`, `minimapResolvedCurrentLine*`) so the child minimap
  layer no longer re-enters editor geometry through callback bindings for those properties.
- `minimapVisualRows`, `minimapScrollable`, `minimapResolvedTrackHeight`, and the viewport/current-line metrics are
  refreshed imperatively from a shared minimap snapshot path instead of `readonly` property bindings. This breaks the
  old cycle where editor layout and minimap layout could repeatedly reopen each other during note snapshot polling.

## Key Collaborators

- `ContentsEditorSelectionBridge`: exposes the selected note body and persistence contracts.
- `ContentsEditorTypingController`: owns ordinary typing/backspace/delete/paste mutation routing from the live editor
  surface back into canonical `.wsnbody` source.
- `ContentsEditorSelectionController`: owns source-range caching, context-menu selection snapshots, and inline-format
  mutation routing for the editor shell.
- `ContentsInlineFormatEditor`: provides the live RichText input surface while preserving the legacy `contentEditor`
  geometry/selection contract consumed by gutter, minimap, and selection helpers.
- `ContentsTextFormatRenderer`: converts inline style tags in editor text into RichText HTML for preview modes.
- `ContentsBodyResourceRenderer`: extracts `<resource ...>` tag render payloads from the selected note body document.
- `ContentsResourceViewer`: renders direct `.wsresource` selections in-place (`image` bitmap / `pdf` reader).
- `ContentsLogicalTextBridge`: maps plain-text offsets to logical line numbers and line starts.
- `ContentsGutterMarkerBridge`: normalizes external marker specifications into a gutter-friendly model.
- `ContentsEditorSession`: owns local-authority tracking, debounced saves, and note-switch synchronization.
- `ContentsGutterLayer` and `ContentsMinimapLayer`: render against the shared geometry helpers exported by this file.
  `ContentsGutterLayer` is mounted only for non-mobile editor surfaces; mobile no longer keeps a zero-value chrome
  placeholder in the row layout.
- `DrawerMenuBar`, `DrawerContents`, and `DrawerToolbar`: compose the lower drawer as separate Figma-aligned modules
  instead of an inline placeholder rectangle.
- Drawer/editor panel partition is computed in one place (`drawerView`):
  - `effectiveDrawerHeight`: clamped runtime drawer height
  - `drawerReservedHeight`: `effectiveDrawerHeight + splitterThickness`
  - `availableDisplayHeight`: `contentsView.height - drawerReservedHeight` (bounded by `minDisplayHeight`)
  - `contentsDisplayView.Layout.preferredHeight` binds to `availableDisplayHeight`, so the drawer never overlays the
    editor surface and the editor never pushes below the drawer frame.

## Interaction and Persistence

- User edits flow through `editorSession.markLocalEditorAuthority()` and `editorSession.scheduleEditorPersistence()`.
- Model-driven note swaps now feed canonical source text into `editorSession.syncEditorTextFromSelection(...)`, while
  `ContentsInlineFormatEditor` binds to `renderedEditorText`.
- Focus-loss persistence now waits for IME preedit to settle before flushing pending editor text, so Hangul syllable
  composition is applied to source text before debounce/save cleanup runs.
- The editor keeps a periodic note snapshot poll (`noteSnapshotRefreshTimer`, default `1200ms`) that calls
  `selectionBridge.refreshSelectedNoteSnapshot()` and schedules gutter refresh passes. This keeps long-running sessions
  synchronized when note metadata/body text changes are delivered late or out-of-band.
- The same refresh path now also recomputes the minimap snapshot so periodic note polling no longer emits repeated QML
  binding-loop warnings for `minimapScrollable`, `minimapResolvedViewportHeight`, or `minimapResolvedViewportY`.
- Cursor movement, scroll position changes, and content-height changes also reschedule that minimap snapshot so the
  viewport thumb and current-line highlight stay live without restoring the old binding cycle.
- The editor viewport now overlays resource cards rendered from `ContentsBodyResourceRenderer.renderedResources`, so image packages referenced in `.wsnbody` are visible without switching to a separate panel.
- When the selected note id is a direct `.wsresource` package, the surface switches to a dedicated in-editor resource
  viewer and hides text-editor/minimap chrome.
- The dedicated resource viewer now fills the entire editor viewport (`anchors.fill: parent`) so no editor inset
  scaffold remains around inline resource rendering.
- Dedicated resource viewer payload selection now prefers `noteListModel.currentResourceEntry` (resource-list
  selection source) and falls back to `ContentsBodyResourceRenderer` only when list payload is unavailable.
- Resource cards are render-mode aware (`image`, `video`, `audio`, `pdf`, `text`, `document`):
  - image: inline thumbnail
  - text: inline snippet preview (`previewText`)
  - non-inline formats: mode label + open action (`Qt.openUrlExternally(...)`)
- The editor viewport also exposes a `DropArea` for file URLs. Drop handling calls
  `resourcesImportViewModel.importUrlsForEditor(...)`, inserts `<resource type=\"...\" format=\"...\" path=...>` tags
  at the cursor position, persists the updated body text, and refreshes `ContentsBodyResourceRenderer` immediately.
- Inline formatting shortcuts persist semantic `.wsnbody` inline tags around the selected text through the save
  pipeline:
  - `Cmd/Ctrl+B` -> `<bold>...</bold>`
  - `Cmd/Ctrl+I` -> `<italic>...</italic>`
  - `Cmd/Ctrl+U` -> `<underline>...</underline>`
  - `Shift+Cmd/Ctrl+X` -> `<strikethrough>...</strikethrough>`
  - `Shift+Cmd/Ctrl+E` -> `<highlight>...</highlight>`
- Reapplying the same shortcut/context-menu style to a selection that is already fully formatted with that same style
  removes the inline formatting and persists plain `.wsnbody` text for that range.
- The editor surface also handles the same shortcuts from `Keys.onPressed` (`Meta/Ctrl + B/I/U`, `Meta/Ctrl+Shift + X/E`)
  so formatting still applies even when platform `Shortcut` dispatch is intercepted by the underlying text input control.
- `Shortcut` bindings are declared with explicit `Meta+...` and `Ctrl+...` sequences (`B/I/U`, `Shift+X`, `Shift+E`)
  to keep desktop key routing deterministic across macOS and non-macOS layouts.
- Right-clicking a non-empty editor selection opens an `LV.ContextMenu` anchored to the editor viewport with:
  - `Plain`
  - `Bold`
  - `Italic`
  - `Underline`
  - `Strikethrough`
  - `Highlight`
- Right-click dispatch is handled by a dedicated `MouseArea` on the editor viewport and opens the menu from
  `onClicked` on the next event-loop turn, avoiding the lost-release path that could suppress the menu.
- Right-click handling now preserves selection intent with a short-lived context-menu snapshot:
  - `contextMenuEditorSelectionRange()` keeps the selection snapshot captured when the context menu opens
  - `inferSelectionRangeFromSelectedText(...)` maps selected plain text back into the current live editor plain text
    when the RichText control does not expose usable numeric offsets.
  - formatting actions no longer wrap raw `.wsnbody` substrings directly. The controller captures the live logical
    selection range, then delegates the actual selected-range mutation to
    `ContentsTextFormatRenderer.applyInlineStyleToLogicalSelectionSource(...)`.
  - formatting actions always require a non-empty selection range before mutating `.wsnbody`
- Context-menu actions dispatch through the same `wrapSelectedEditorTextWithTag(...)` path as keyboard shortcuts, so
  inline tag serialization/persistence behavior stays identical across input methods.
- The top-level `Plain` context-menu action clears all inline formatting from the selected range and persists the
  resulting plain `.wsnbody` text.
- Shortcut enablement no longer hard-gates on `editorInputFocused`; instead, shortcuts stay available while a note is
  active, but the actual wrap request still captures the live `TextEdit` selection snapshot before the queued mutation
  runs.
- The focus probe still tracks `contentEditor.focused` / `activeFocus`, `editorItem.activeFocus`, and
  `editorItem.inputItem.activeFocus` for marker/interaction state decisions.
- The editor now resolves `editorViewModeViewModel` (from injected property or `LV.ViewModels`) and switches renderer
  surfaces by mode:
  - `Plain` (`activeViewMode == 0`):
    - `ContentsInlineFormatEditor` RichText editing surface (cursor/input always enabled for note editing).
  - `Page` (`activeViewMode == 1`): paper-layout RichText editing surface (A4 portrait scaffold, no print guides)
  - `Print` (`activeViewMode == 2`): paper-layout RichText editing surface + dashed print-margin guides
  - `Web/Presentation`: same editable RichText surface, keeping document editing consistent across all editor views.
- `Page`/`Print` paper scaffold keeps a fixed A4 aspect ratio (`210 / 297`) with width-first fitting
  (`printEditorPage.availableWidth`) and no height-fit shrink pass, so the paper size no longer collapses to match
  viewport height.
- `Page`/`Print` text layout now maps the editor frame directly to the printable rectangle inside the page:
  `left/right/top/bottom` margins are offset by `printGuideHorizontalInset` / `printGuideVerticalInset`, and
  editor-side `insetHorizontal` / `insetVertical` are pinned to `0` in paper layouts.
- This keeps the first rendered line below the top print margin and prevents text from crossing the page guide even
  when the viewport height changes.
- `Print` mode draws dashed margin guides for that printable area; `Page` mode uses the same margins for text layout
  but intentionally hides guide lines.
- `Page`/`Print` text color is explicitly forced to black (`#000000`) for editor and formatted preview surfaces so
  print/paper rendering does not inherit low-contrast dark-theme text colors.
- `showFormattedTextRenderer` is intentionally pinned to `false`; the legacy read-only replacement layer remains
  disabled so the editor never drops into a non-editable state.
- `ContentsInlineFormatEditor` now owns the live RichText surface, and the control receives `renderedEditorText`
  instead of the raw `.wsnbody` source string.
- Direct typing in the RichText surface no longer round-trips the full RichText document back through
  `normalizeEditorSurfaceTextToSource(...)`.
- Instead, `onTextEdited` delegates to `ContentsEditorTypingController`, which:
  - compares the current live editor plain text to `ContentsLogicalTextBridge.logicalText`
  - computes one contiguous plain-text replacement delta
  - maps that logical delta back to source offsets
  - replaces only that source span in `.wsnbody`
- Whole-document RichText normalization is now reserved for formatting application (`QTextDocument/QTextCursor`)
  rather than ordinary typing.
- The view still reapplies `TextEdit.RichText` / `showRenderedOutput` across the editor wrapper, its inner editor
  item, and the nested input item after note/session sync so the editable surface does not fall back to plain-text
  rendering.
- Stored inline aliases (`<bold>`, `<italic>`, `<underline>`, `<strikethrough>`, `<highlight>`, `<mark>`) are
  normalized into editable RichText spans before session sync:
  - `bold` -> `<strong style=\"font-weight:900;\">`
  - `italic` -> `<span style=\"font-style:italic;\">`
  - `underline` -> `<span style=\"text-decoration: underline;\">`
  - `strikethrough` -> `<span style=\"text-decoration: line-through;\">`
  - `highlight`/`mark` -> dark-orange background + LVRS `accentYellowMuted` foreground styled `<span ...>`
- The same renderer path now also styles markdown-like note input directly in the editable surface:
    - `1. ` / `2. ` / `3) ` -> numbered-list line styling
    - `- ` / `* ` / `+ ` -> unordered-list line styling with a rendered bullet glyph
    - `#` ... `######` -> heading-like line styling
    - `> ` -> blockquote-like line styling
    - `` `code` ``, `[label](url)`, and fenced code blocks -> code/link-styled literals
- Those markdown presentation roles are emitted through the shared `WhatSonNoteMarkdownStyleObject`, so renderer output
  and `.wsnbody` promotion rules stay aligned.
- Bold, italic, underline, strikethrough, and highlight remain proprietary `.wsnbody` inline-format tags; markdown
  emphasis markers are intentionally not the authoritative formatting path.
- Alias normalization is delegated to `ContentsTextFormatRenderer.normalizeInlineStyleAliasesForEditor(...)` from
  `normalizeBodySourceForRichTextEditor(...)`, so `.wsnbody` inline tags are normalized through the renderer contract
  before entering the editable surface.
- That normalization path now promotes canonical source `\n` into explicit RichText `<br/>` breaks before the value is
  rebound into `ContentsInlineFormatEditor`.
- The surrounding persistence path preserves leading/trailing whitespace-only lines and empty paragraphs so intentional
  blank-line spacing created in the editor survives save/load round-trips.
- Escaped safe text such as `&lt;bold&gt;...&lt;/bold&gt;` is intentionally preserved as literal text (not decoded).
- As a result, inline formatting is rendered directly inside the editable editor surface instead of showing raw style
  tag text, even when the LVRS shell component does not provide reliable RichText styling on its own.

## Tests

- Automated test files are not currently present in this repository.
- Editor formatting regression checklist for this file:
  - ordinary typing/backspace/delete/paste must not serialize fragment comment markup such as `<!--StartFragment-->`
    back into `.wsnbody`
  - typing Hangul in the note body must not randomly delete surrounding text when a syllable block commits
  - typing Hangul in the note body must not leave separated jamo after the committed syllable appears
  - pressing `Enter` in the editable RichText surface must persist and re-render as an actual line break, not as a
    space or adjacent text collapse
  - opening a note and waiting through repeated note snapshot refreshes must not emit repeated minimap binding-loop
    warnings
  - scrolling the editor or moving the cursor must keep the minimap viewport thumb and current-line highlight in sync
  - gutter line numbers and current-line markers must remain aligned to the first visible text row after whitespace /
    RichText normalization changes
  - existing `.wsnbody` `<bold>` regions render as visible bold text on load
  - typing `- item` must re-render as a bullet-list line in the editor surface
  - typing `1. item` must re-render as a numbered-list line in the editor surface
  - typing `# heading` or `> quote` must re-render with heading/blockquote styling without changing the stored source
    marker characters
  - fenced code blocks and inline code/link-shaped literals must re-render with markdown-aware styling while preserving
    their literal source text
  - `Cmd/Ctrl+B` on heading text, `Cmd/Ctrl+I` on blockquote text, `Cmd/Ctrl+U` on link literals, and highlight on code
    literals must still operate on proprietary `.wsnbody` tags rather than treating markdown presentation as the
    already-applied shortcut state
  - right-clicking a non-empty selection opens the formatting context menu
  - the context menu shows `Plain` as the first item
  - context-menu formatting wraps the actual selected source span, not a duplicated plain-text occurrence elsewhere
  - choosing `Plain` removes inline tags from the selected source span
  - reapplying the same formatting action to an already formatted selection restores that selection to plain text
  - opening an empty note must not render a `Start typing here` placeholder overlay
  - desktop editor text must render at `13px`
  - mobile editor text must render `2px` larger than desktop
  - mobile editor route must not reserve or render the gutter column
- In `Page`/`Print`, the preview text geometry is aligned to `printEditorPage` and reuses the same guide inset math as
  the editor surface, so wrapped text width and top offset match the printable rectangle.
- Mutation surfaces (`DropArea`, edit shortcuts, gutter/minimap) remain active because the editor is intentionally
  always editable for note-taking workflows.
- An empty note no longer renders a `Start typing here` overlay; the editor surface stays visually blank until the
  user types real content.
- `focusEditorForPendingNote()` moves focus and cursor placement after note creation or route changes resolve.
- `drawerQuickNoteText` is a local drawer draft state for the inline Quick Note page. The drawer forwards toolbar and
  mode actions back through `requestViewHook(...)` so the panel-level owner can attach real behavior later.
- Repository policy note: automated test files were removed; this document therefore treats the checklist above as the
  expected regression surface for later runtime/manual verification.

## Scroll and Minimap Rules

- Minimap viewport math uses the outer editor viewport height, not the inner LVRS flickable height, because the shared
  top spacer is part of the visible editor frame.
- Scroll-to-minimap routing uses `editorOccupiedContentHeight()` so the minimap and gutter continue to agree on the
  same content span even after the fixed top spacer is reserved above the text surface.
- `ContentsMinimapLayer` now receives viewport visibility/height/Y and current-line highlight geometry as resolved
  numbers, not callback resolvers. This prevents the child layer from reopening layout calculations while its own
  rectangles are being bound.
