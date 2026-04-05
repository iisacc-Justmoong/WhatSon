# `src/app/qml/view/panels/navigation`

## Status
- Directory mirror generated from the current `src` tree.
- This file is the entry point for the detailed documentation pass of this directory.

## Scope
- Mirrored source directory: `src/app/qml/view/panels/navigation`
- Child directories: 3
- Child files: 10

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
- `NavigationEditorViewBar.qml`
- `NavigationInformationBar.qml`
- `NavigationModeBar.qml`
- `NavigationPreferenceBar.qml`
- `NavigationPropertiesBar.qml`

## Recent Notes
- The small shared navigation bars now use `LV.Theme.gap...` tokens for their inter-button spacing instead of local
  integer literals such as `2`, `4`, `8`, or `12`.
- `NavigationEditorViewBar.qml` also moved its popup offset and menu width to LVRS token/scale-aware metrics.

## Intended Detailed Sections
- Module responsibilities and architectural layer
- Internal submodule boundaries
- Cross-directory dependencies
- Runtime ownership and lifecycle rules
- Testing strategy and coverage map
- Known hotspots and refactor priorities
