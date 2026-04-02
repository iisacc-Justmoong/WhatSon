# `src/app/qml/view/content/editor/ContentsDisplayView.qml`

## Responsibility

`ContentsDisplayView.qml` is the shared editor surface for desktop and mobile note editing. It composes the LVRS
`TextEditor`, gutter, minimap, and lower drawer into one geometry contract so every collaborator can resolve line
positions from the same document origin.

## Geometry Contract

- `editorDocumentStartY` is the canonical top inset for the editable document.
- The visible `16px` top spacer is materialized through the outer `LV.TextEditor` top margin, not through a late
  mutation of `TextEditor.editorItem.y`.
- `editorDocumentTopPadding` remains `0` so LVRS vertical centering does not reintroduce hidden top padding inside the
  text area.
- `editorSurfaceHeight` is derived from `editorViewportHeight - editorDocumentStartY`, which keeps Fill sizing stable
  after reserving the fixed top spacer.
- `logicalTextLength()` is the canonical line-metric length source for gutter/minimap cursor geometry in RichText mode.
  It is derived from `ContentsLogicalTextBridge` logical offsets/counts instead of raw `editorText.length`, so markup
  token length cannot desynchronize gutter row placement.
- `logicalLineDocumentYCache` stores monotonic document-Y values per logical line. It is rebuilt per
  `gutterRefreshRevision` and reused by both `lineDocumentY(...)` and
  `logicalLineNumberForDocumentY(...)` so binary-search visibility and rendered gutter delegates never drift into
  overlapping or skipped line-number rows when empty lines/wrapped edits jitter `positionToRectangle(...)`.

## Key Collaborators

- `ContentsEditorSelectionBridge`: exposes the selected note body and persistence contracts.
- `ContentsTextFormatRenderer`: converts inline style tags in editor text into RichText HTML for preview modes.
- `ContentsBodyResourceRenderer`: extracts `<resource ...>` tag render payloads from the selected note body document.
- `ContentsResourceViewer`: renders direct `.wsresource` selections in-place (`image` bitmap / `pdf` reader).
- `ContentsLogicalTextBridge`: maps plain-text offsets to logical line numbers and line starts.
- `ContentsGutterMarkerBridge`: normalizes external marker specifications into a gutter-friendly model.
- `ContentsEditorSession`: owns local-authority tracking, debounced saves, and note-switch synchronization.
- `ContentsGutterLayer` and `ContentsMinimapLayer`: render against the shared geometry helpers exported by this file.
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
- Model-driven note swaps call `editorSession.syncEditorTextFromSelection(...)` and flush pending edits when the bound
  note changes.
- The editor keeps a periodic note snapshot poll (`noteSnapshotRefreshTimer`, default `1200ms`) that calls
  `selectionBridge.refreshSelectedNoteSnapshot()` and schedules gutter refresh passes. This keeps long-running sessions
  synchronized when note metadata/body text changes are delivered late or out-of-band.
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
- Inline formatting shortcuts wrap selected text with editable RichText tags:
  - `Cmd/Ctrl+B` -> `<strong style=\"font-weight:700;\">...</strong>`
  - `Cmd/Ctrl+I` -> `<i>...</i>`
  - `Cmd/Ctrl+U` -> `<u>...</u>`
  - `Shift+Cmd/Ctrl+X` -> `<s>...</s>`
  - `Shift+Cmd/Ctrl+H` -> `<span style=\"background-color:#8A4B00;color:#FFD9A3;font-weight:600;\">...</span>`
- The editor surface also handles the same shortcuts from `Keys.onPressed` (`Meta/Ctrl + B/I/U`, `Meta/Ctrl+Shift + X/H`)
  so formatting still applies even when platform `Shortcut` dispatch is intercepted by the underlying text input control.
- Right-clicking a non-empty editor selection opens an `LV.ContextMenu` anchored to the editor viewport with:
  - `Bold`
  - `Italic`
  - `Underline`
  - `Strikethrough`
  - `Highlight`
- Right-click handling now preserves selection intent by caching the last non-empty selection range:
  - `cachedEditorSelectionRange()` keeps the latest valid selection
  - `contextMenuEditorSelectionRange()` keeps the selection snapshot captured when the context menu opens
  - formatting actions always require a non-empty selection range before mutating `.wsnbody`
- Context-menu actions dispatch through the same `wrapSelectedEditorTextWithTag(...)` path as keyboard shortcuts, so
  inline tag serialization/persistence behavior stays identical across input methods.
- Shortcut enablement no longer hard-gates on `editorInputFocused`; instead, shortcuts stay available while a note is
  active and `wrapSelectedEditorTextWithTag(...)` enforces the non-empty selection guard.
- The focus probe still tracks `LV.TextEditor.focused` / `activeFocus`, `editorItem.activeFocus`, and
  `editorItem.inputItem.activeFocus` for marker/interaction state decisions.
- The editor now resolves `editorViewModeViewModel` (from injected property or `LV.ViewModels`) and switches renderer
  surfaces by mode:
  - `Plain` (`activeViewMode == 0`):
    - `LV.TextEditor` RichText editing surface (cursor/input always enabled for note editing).
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
- `LV.TextEditor.showRenderedOutput` is enabled, so inline tags normalized into RichText aliases (`<strong>`, `<em>`,
  `<u>`, `<s>`, highlight `<span>`) render with style directly inside the editable surface.
- Stored inline aliases (`<bold>`, `<italic>`, `<underline>`, `<strikethrough>`, `<highlight>`, `<mark>`) are
  normalized into editable RichText tags before session sync:
  - `bold` -> `<strong style=\"font-weight:700;\">`
  - `italic` -> `<i>`
  - `underline` -> `<u>`
  - `strikethrough` -> `<s>`
  - `highlight`/`mark` -> orange styled `<span ...>`
- Alias normalization is delegated to `ContentsTextFormatRenderer.normalizeInlineStyleAliasesForEditor(...)` from
  `normalizeBodySourceForRichTextEditor(...)`, so `.wsnbody` inline tags are normalized through the renderer contract
  before entering the editable surface.
- Escaped safe text such as `&lt;bold&gt;...&lt;/bold&gt;` is intentionally preserved as literal text (not decoded).
- As a result, inline formatting is rendered directly inside the editable editor surface instead of showing raw style
  tag text.
- In `Page`/`Print`, the preview text geometry is aligned to `printEditorPage` and reuses the same guide inset math as
  the editor surface, so wrapped text width and top offset match the printable rectangle.
- Mutation surfaces (`DropArea`, edit shortcuts, gutter/minimap) remain active because the editor is intentionally
  always editable for note-taking workflows.
- `focusEditorForPendingNote()` moves focus and cursor placement after note creation or route changes resolve.
- `drawerQuickNoteText` is a local drawer draft state for the inline Quick Note page. The drawer forwards toolbar and
  mode actions back through `requestViewHook(...)` so the panel-level owner can attach real behavior later.

## Scroll and Minimap Rules

- Minimap viewport math uses the outer editor viewport height, not the inner LVRS flickable height, because the shared
  top spacer is part of the visible editor frame.
- Scroll-to-minimap routing uses `editorOccupiedContentHeight()` so the minimap and gutter continue to agree on the
  same content span even after the fixed top spacer is reserved above the text surface.

## Tests

- `tests/app/test_qml_binding_syntax_guard.cpp` guards the shared top inset, zero top padding, Fill-height contract,
  editor-session wiring, and the three-part quick-note drawer composition.
