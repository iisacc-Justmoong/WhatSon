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
- Mobile scroll momentum is now treated as a panel-level interaction contract: `ListBarLayout.qml` preserves native
  kinetic carry for touch scrolling, and `SidebarHierarchyView.qml` explicitly pushes LVRS hierarchy scroll surfaces
  onto the mobile flick profile.

## Intended Detailed Sections
- Module responsibilities and architectural layer
- Internal submodule boundaries
- Cross-directory dependencies
- Runtime ownership and lifecycle rules
- Testing strategy and coverage map
- Known hotspots and refactor priorities
