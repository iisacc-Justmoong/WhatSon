# `src/app/qml/view/panels/navigation/view`

## Status
- Directory mirror generated from the current `src` tree.
- This file is the entry point for the detailed documentation pass of this directory.

## Scope
- Mirrored source directory: `src/app/qml/view/panels/navigation/view`
- Child directories: 0
- Child files: 4

## Child Directories
- No child directories.

## Child Files
- `NavigationApplicationViewCalendarBar.qml`
- `NavigationApplicationViewBar.qml`
- `NavigationApplicationViewModeBar.qml`
- `NavigationApplicationViewOptionBar.qml`

## Recent Notes
- `NavigationApplicationViewBar.qml` now mirrors the Figma metadata order
  `ViewOptionBar -> ModeBar -> CalendarBar -> AddNewBar -> PreferenceBar`.
- View-only icon contracts that diverged from the shared navigation bars now live in local files under this
  directory so edit/control mode wrappers stay isolated.
- The view-option `Center View` action now uses LVRS `recursiveMethod`, while the mode-level `Center View Mode`
  action keeps LVRS `singleRecordView`; those two Figma glyphs are intentionally different.

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

- Target:  ``src/app/qml/view/panels/navigation/view``  ( `docs/src/app/qml/view/panels/navigation/view/README.md` )
- Location:  `docs/src/app/qml/view/panels/navigation/view`
- Role: This file describes the structure, responsibility, operating rules, and validation criteria of the corresponding directory or module.
- Criteria: File path, command,  API  name, and detailed change history are maintained based on the original English text.
- On Change: If the above English text is modified, this Korean lower section is also updated to the latest state.
