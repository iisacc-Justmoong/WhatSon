# `src/app/qml/view/panels`

## Status
- Directory mirror generated from the current `src` tree.
- This file is the entry point for the detailed documentation pass of this directory.

## Scope
- Mirrored source directory: `src/app/qml/view/panels`
- Child directories: 4
- Child files: 12

## Child Directories
- `detail`
- `list`
- `navigation`
- `sidebar`

## Child Files
- `BodyLayout.qml`
- `ContentViewLayout.qml`
- `DetailPanelLayout.qml`
- `HierarchySidebarLayout.qml`
- `ListBarHeader.qml`
- `ListBarLayout.qml`
- `NavigationBarLayout.qml`
- `NoteListItem.qml`
- `PanelEdgeSplitter.qml`
- `ResourceListItem.qml`
- `StatusBarLayout.qml`

## Recent Notes
- The current workspace route mounts the restored panel shell. `ContentViewLayout.qml` is the center content slot and
  now composes image resource viewing through `ImageEditor.qml` plus a blank placeholder for note and non-image
  selections. The deleted note editor toolbar and text surface must not be reconstructed in QML.
  kinetic carry for touch scrolling, and `SidebarHierarchyView.qml` explicitly pushes LVRS hierarchy scroll surfaces

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

- Target:  ``src/app/qml/view/panels``  ( `docs/src/app/qml/view/panels/README.md` )
- Location:  `docs/src/app/qml/view/panels`
- Role: This file describes the structure, responsibility, operating rules, and validation criteria of the corresponding directory or module.
- The current workspace route mounts the existing panel shell, and the center content slot combines only the image resource viewer and empty content placeholder through `ContentViewLayout.qml`. The deleted note editor toolbar and text surface are not recreated in QML.
- Criteria: File path, command,  API  name, and detailed change history are maintained based on the original English text.
- On Change: If the above English text is modified, this Korean lower section is also updated to the latest state.
