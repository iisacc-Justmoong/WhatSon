# `src/app/qml/view/panels/navigation`

## Status
- Directory mirror generated from the current `src` tree.
- This file is the entry point for the detailed documentation pass of this directory.

## Scope
- Mirrored source directory: `src/app/qml/view/panels/navigation`
- Child directories: 3
- Child files: 9

## Child Directories
- `control`
- `edit`
- `view`

## Child Files
- `NavigationAddNewBar.qml`
- `NavigationApplicationAddNewBar.qml`
- `NavigationApplicationCalendarBar.qml`
- `NavigationApplicationPreferenceBar.qml`
- `NavigationCalendarBar.qml`
- `NavigationInformationBar.qml`
- `NavigationModeBar.qml`
- `NavigationPreferenceBar.qml`
- `NavigationPropertiesBar.qml`

## Recent Notes
- The small shared navigation bars now use `LV.Theme.gap...` tokens for their inter-button spacing instead of local
  integer literals such as `2`, `4`, `8`, or `12`.
- The removed editor view-mode chrome stays absent; navigation bars must not recreate Plain/Page/Print/Web/Presentation
  selection without a new document model contract.

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

- Target:  ``src/app/qml/view/panels/navigation``  ( `docs/src/app/qml/view/panels/navigation/README.md` )
- Location:  `docs/src/app/qml/view/panels/navigation`
- Role: This file describes the structure, responsibility, operating rules, and validation criteria of the corresponding directory or module.
- Deleted editor view-mode chrome is not revived.
- Criteria: File path, command,  API  name, and detailed change history are maintained based on the original English text.
- On Change: If the above English text is modified, this Korean lower section is also updated to the latest state.
