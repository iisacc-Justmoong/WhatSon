# `src/app/qml/view/content/editor`

## Status
- Directory mirror generated from the current `src` tree.
- This file is the entry point for the detailed documentation pass of this directory.

## Scope
- Mirrored source directory: `src/app/qml/view/content/editor`
- Child directories: 0
- Child files: 13

## Child Directories
- No child directories.

## Child Files
- `ContentsDisplayView.qml`
- `ContentsDrawerSplitter.qml`
- `ContentsEditorSelectionController.qml`
- `ContentsEditorSession.qml`
- `ContentsEditorTypingController.qml`
- `ContentsGutterLayer.qml`
- `ContentsInlineFormatEditor.qml`
- `ContentsMinimapLayer.qml`
- `MobileContentsDisplayView.qml`
- `ContentsResourceViewer.qml`
- `DrawerContents.qml`
- `DrawerMenuBar.qml`
- `DrawerToolbar.qml`

## Current Notes

- The lower editor drawer is now decomposed into three dedicated sibling QML files that mirror the Figma frames:
  `DrawerMenuBar.qml` (`155:4565`), `DrawerContents.qml` (`174:6352`), and `DrawerToolbar.qml` (`155:4570`).
- `ContentsDisplayView.qml` is now the desktop-only editor surface.
- `MobileContentsDisplayView.qml` is the duplicated mobile-only editor surface, so gutter/font behavior is no longer
  gated through one shared desktop/mobile file.
- `ContentsEditorTypingController.qml` now owns ordinary text-entry mutation routing so typing no longer reserializes
  the whole RichText surface on every edit.
- Markdown list continuation on `Enter` is also owned by `ContentsEditorTypingController.qml`, so bullet/numbered list
  continuation stays source-driven instead of relying on RichText widget heuristics or the editor's rendered bullet
  glyph representation.
- `Page` / `Print` now mount the live RichText editor inside an outer paper-document viewport, so the paper grows with
  the note instead of remaining a fixed-height scaffold.
- The repository no longer operates scripted editor tests; the per-file regression notes in this directory are
  documentation-only behavior contracts.

## Intended Detailed Sections
- Module responsibilities and architectural layer
- Internal submodule boundaries
- Cross-directory dependencies
- Runtime ownership and lifecycle rules
- Testing strategy and coverage map
- Known hotspots and refactor priorities
