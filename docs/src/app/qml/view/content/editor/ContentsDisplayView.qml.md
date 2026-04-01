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

## Key Collaborators

- `ContentsEditorSelectionBridge`: exposes the selected note body and persistence contracts.
- `ContentsBodyResourceRenderer`: extracts `<resource ...>` tag render payloads from the selected note body document.
- `ContentsResourceViewer`: renders direct `.wsresource` selections in-place (`image` bitmap / `pdf` reader).
- `ContentsLogicalTextBridge`: maps plain-text offsets to logical line numbers and line starts.
- `ContentsGutterMarkerBridge`: normalizes external marker specifications into a gutter-friendly model.
- `ContentsEditorSession`: owns local-authority tracking, debounced saves, and note-switch synchronization.
- `ContentsGutterLayer` and `ContentsMinimapLayer`: render against the shared geometry helpers exported by this file.
- `DrawerMenuBar`, `DrawerContents`, and `DrawerToolbar`: compose the lower drawer as separate Figma-aligned modules
  instead of an inline placeholder rectangle.

## Interaction and Persistence

- User edits flow through `editorSession.markLocalEditorAuthority()` and `editorSession.scheduleEditorPersistence()`.
- Model-driven note swaps call `editorSession.syncEditorTextFromSelection(...)` and flush pending edits when the bound
  note changes.
- The editor viewport now overlays resource cards rendered from `ContentsBodyResourceRenderer.renderedResources`, so image packages referenced in `.wsnbody` are visible without switching to a separate panel.
- When the selected note id is a direct `.wsresource` package, the surface switches to a dedicated in-editor resource
  viewer and hides text-editor/minimap chrome.
- Resource cards are render-mode aware (`image`, `video`, `audio`, `pdf`, `text`, `document`):
  - image: inline thumbnail
  - text: inline snippet preview (`previewText`)
  - non-inline formats: mode label + open action (`Qt.openUrlExternally(...)`)
- The editor viewport also exposes a `DropArea` for file URLs. Drop handling calls
  `resourcesImportViewModel.importUrlsForEditor(...)`, inserts `<resource type=\"...\" format=\"...\" path=...>` tags
  at the cursor position, persists the updated body text, and refreshes `ContentsBodyResourceRenderer` immediately.
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
