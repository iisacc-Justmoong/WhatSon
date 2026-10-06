# `src/app/qml/view/panels/detail`

## Status
- Directory mirror generated from the current `src` tree.
- This file is the entry point for the detailed documentation pass of this directory.

## Scope
- Mirrored source directory: `src/app/qml/view/panels/detail`
- Child directories: 0
- Child files: 11

## Child Directories
- No child directories.

## Child Files
- `DetailContents.qml`
- `DetailFileStatForm.qml`
- `DetailMetadataHierarchyPicker.qml`
- `DetailMetadataSelectionController.qml`
- `CalendarDetailPanel.qml`
- `DetailPanel.qml`
- `DetailPanelHeaderToolbar.qml`
- `DetailPanelHeaderToolbarButton.qml`
- `NoteDetailPanel.qml`
- `ResourceDetailPanel.qml`
- `RightPanel.qml`

## Recent Notes
- `DetailPanel.qml` is now only the route-aware router for the detail column, and it switches between
  `CalendarDetailPanel.qml`, `NoteDetailPanel.qml`, and `ResourceDetailPanel.qml` instead of forcing one note-only
  form onto every hierarchy or calendar route.
- `CalendarDetailPanel.qml` is intentionally blank for now, but it already owns a dedicated calendar-detail surface so
  future calendar-only UI can grow without reopening note/resource branching.
- `ResourceDetailPanel.qml` is intentionally blank for now, but it already owns a dedicated resource-detail
  controller contract so future resource-only UI can grow without reopening note-detail branching.
  surfaces must remain LVRS scale-aware instead of assuming desktop `1.0x` metrics.
  same hierarchy rendering contract through an overridden `LV.ContextMenu`, and the picker body now renders the full
  hierarchy as a permanently expanded `LV.HierarchyItem` list instead of an embeddable tree panel.

## Intended Detailed Sections
- Module responsibilities and architectural layer
- Internal submodule boundaries
- Cross-directory dependencies
- Runtime ownership and lifecycle rules
- Testing strategy and coverage map
- Known hotspots and refactor priorities

<a id="한국어"></a>

## Korean

This section is a bottom summary for checking the above  README  content in Korean.

- Target:  ``src/app/qml/view/panels/detail``  ( `docs/src/app/qml/view/panels/detail/README.md` )
- Location:  `docs/src/app/qml/view/panels/detail`
- Role: This file describes the structure, responsibility, operating rules, and validation criteria of the corresponding directory or module.
- Criteria: File path, command,  API  name, and detailed change history are maintained based on the original English text.
- On Change: If the above English text is modified, this Korean lower section is also updated to the latest state.
