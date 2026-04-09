# `src/app/qml/view/panels/detail`

## Status
- Directory mirror generated from the current `src` tree.
- This file is the entry point for the detailed documentation pass of this directory.

## Scope
- Mirrored source directory: `src/app/qml/view/panels/detail`
- Child directories: 0
- Child files: 8

## Child Directories
- No child directories.

## Child Files
- `DetailContents.qml`
- `DetailFileStatForm.qml`
- `DetailMetadataHierarchyPicker.qml`
- `DetailMetadataSelectionController.qml`
- `DetailPanel.qml`
- `DetailPanelHeaderToolbar.qml`
- `DetailPanelHeaderToolbarButton.qml`
- `RightPanel.qml`

## Recent Notes
- `DetailContents.qml` is shared by desktop and mobile detail routes, so compact section spacing and fixed-height
  surfaces must remain LVRS scale-aware instead of assuming desktop `1.0x` metrics.
- `DetailMetadataHierarchyPicker.qml` now owns the shared folder/tag add overlay so desktop and mobile reuse the
  same hierarchy rendering contract through an overridden `LV.ContextMenu`, and the picker body now renders the full
  hierarchy as a permanently expanded `LV.HierarchyItem` list instead of an embeddable tree panel.

## Intended Detailed Sections
- Module responsibilities and architectural layer
- Internal submodule boundaries
- Cross-directory dependencies
- Runtime ownership and lifecycle rules
- Testing strategy and coverage map
- Known hotspots and refactor priorities
