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
- `ContentsLogicalTextBridge`: maps plain-text offsets to logical line numbers and line starts.
- `ContentsGutterMarkerBridge`: normalizes external marker specifications into a gutter-friendly model.
- `ContentsEditorSession`: owns local-authority tracking, debounced saves, and note-switch synchronization.
- `ContentsGutterLayer` and `ContentsMinimapLayer`: render against the shared geometry helpers exported by this file.

## Interaction and Persistence

- User edits flow through `editorSession.markLocalEditorAuthority()` and `editorSession.scheduleEditorPersistence()`.
- Model-driven note swaps call `editorSession.syncEditorTextFromSelection(...)` and flush pending edits when the bound
  note changes.
- `focusEditorForPendingNote()` moves focus and cursor placement after note creation or route changes resolve.

## Scroll and Minimap Rules

- Minimap viewport math uses the outer editor viewport height, not the inner LVRS flickable height, because the shared
  top spacer is part of the visible editor frame.
- Scroll-to-minimap routing uses `editorOccupiedContentHeight()` so the minimap and gutter continue to agree on the
  same content span even after the fixed top spacer is reserved above the text surface.

## Tests

- `tests/app/test_qml_binding_syntax_guard.cpp` guards the shared top inset, zero top padding, Fill-height contract,
  and editor-session wiring.
