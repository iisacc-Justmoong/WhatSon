# `src/app/qml/view/panels/StatusBarLayout.qml`

## Status
- Documentation phase: scaffold generated from the live source tree.
- Detail level: structural placeholder prepared for a later deep pass.

## Source Metadata
- Source path: `src/app/qml/view/panels/StatusBarLayout.qml`
- Source kind: QML view/component
- File name: `StatusBarLayout.qml`
- Approximate line count: 195

## QML Surface Snapshot
- Root type: `Rectangle`

## Current Theme Contract
- Desktop `searchFieldColor` is intentionally transparent so the status strip inherits the root `ApplicationWindow`
  canvas instead of painting a second panel slab behind the search affordance.
- Mobile compact search keeps its own filled field token, but the shared mobile scaffold now feeds that token from the
  same root canvas tone instead of a brighter mobile-only control surface color.
- The desktop search-field min/max width clamps now route through `LV.Theme.scaleMetric(220/541)` instead of raw pixel
  literals, so wide/narrow status-bar search remains LVRS density-aware.

### Object IDs
- `statusBar`
- `searchBarTextField`
- `searchBarInput`
- `windowControlsHitArea`
- `windowControlIconsHitArea`
- `compactStatusBar`
- `compactSearchBar`
- `compactSearchInput`
- `newFileButtonSlot`
- `newFileButton`

### Required Properties
- None detected during scaffold generation.

### Signals
- `searchSubmitted`
- `searchTextEdited`
- `createNoteRequested`
- `viewHookRequested`
- `windowMoveRequested`

## Intended Detailed Sections
- Responsibility and business role
- Ownership and lifecycle
- Public API or externally observed bindings
- Collaborators and dependency direction
- Data flow and state transitions
- Error handling and recovery paths
- Threading, scheduling, or UI affinity constraints when relevant
- Extension points, invariants, and known complexity hotspots
- Test coverage and missing verification

## Authoring Notes For Next Pass
- Read the real implementation and adjacent headers before replacing this scaffold.
- Document concrete signals, slots, invokables, persistence side effects, and LVRS/QML bindings where applicable.
- Cross-link this file with peer modules in the same directory once the detailed pass begins.
