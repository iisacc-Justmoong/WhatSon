# `src/app/qml/view/content/editor/ContentsDisplayView.qml`

## Responsibility

`ContentsDisplayView.qml` is the desktop editor surface for note contents.
It owns the desktop-only gutter/minimap presentation, the shared rich-text editing surface, and the resource overlays
that sit directly inside the editor viewport.

## Current Layout Contract

- The editor surface now fills the entire available content slot.
- The root surface keeps `displayColor` as the only background fill for the editor area.
- The desktop gutter remains visible when the selected surface is an editable note body.
- The minimap remains desktop-only and is mounted beside the editor viewport.
- `Page` / `Print` mode still route the live editor through the outer paper-document viewport so scrolling owns the
  paper surface rather than a fixed-height nested editor.

## Key Collaborators

- `ContentsInlineFormatEditor.qml`
- `ContentsEditorSelectionController.qml`
- `ContentsEditorTypingController.qml`
- `ContentsEditorSession.qml`
- `ContentsGutterLayer.qml`
- `ContentsMinimapLayer.qml`
- `ContentsResourceViewer.qml`
- `ContentViewLayout.qml`

## Interaction Notes

- The editor stays editable in all supported content modes; the legacy formatted-preview fallback remains disabled.
- Resource cards rendered from `<resource ...>` tags still overlay the editor viewport when the selected note body
  references inline assets.
- Direct `.wsresource` selections still switch the surface to the dedicated in-editor resource viewer.
- Context-menu formatting, keyboard shortcuts, gutter refresh, and minimap snapshot refresh all remain rooted in this
  file.
- Desktop window shortcuts mirror the selection controller contract for markdown lists:
  - macOS: `Cmd+Shift+7` / `Cmd+Shift+8`
  - Windows/Linux: `Alt+Shift+7` / `Alt+Shift+8`

## Tests

- Automated test files are not currently present in this repository.
- Regression checklist:
  - Desktop contents must render without reserving any extra bottom partition height.
  - Gutter line numbers and minimap geometry must still align with the live editor surface.
  - Resource overlays and dedicated resource viewing must still occupy the editor viewport correctly.
  - `Page` / `Print` mode must keep the external paper-document scroll contract.
  - Desktop markdown list shortcuts (`Cmd+Shift+7/8` on macOS, `Alt+Shift+7/8` on Windows/Linux) must still reach the
    selection controller while the rich-text editor owns focus.
