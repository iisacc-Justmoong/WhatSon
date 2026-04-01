# `src/app/qml/view/content/editor`

## Status
- Directory mirror generated from the current `src` tree.
- This file is the entry point for the detailed documentation pass of this directory.

## Scope
- Mirrored source directory: `src/app/qml/view/content/editor`
- Child directories: 0
- Child files: 9

## Child Directories
- No child directories.

## Child Files
- `ContentsDisplayView.qml`
- `ContentsDrawerSplitter.qml`
- `ContentsEditorSession.qml`
- `ContentsGutterLayer.qml`
- `ContentsMinimapLayer.qml`
- `ContentsResourceViewer.qml`
- `DrawerContents.qml`
- `DrawerMenuBar.qml`
- `DrawerToolbar.qml`

## Current Notes

- The lower editor drawer is now decomposed into three dedicated sibling QML files that mirror the Figma frames:
  `DrawerMenuBar.qml` (`155:4565`), `DrawerContents.qml` (`174:6352`), and `DrawerToolbar.qml` (`155:4570`).
- `ContentsDisplayView.qml` remains the integration point and keeps local draft state plus panel-view-model hook routing
  for the new drawer modules.

## Intended Detailed Sections
- Module responsibilities and architectural layer
- Internal submodule boundaries
- Cross-directory dependencies
- Runtime ownership and lifecycle rules
- Testing strategy and coverage map
- Known hotspots and refactor priorities
