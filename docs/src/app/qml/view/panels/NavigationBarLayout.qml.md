# `src/app/qml/view/panels/NavigationBarLayout.qml`

## Status
- Documentation phase: scaffold generated from the live source tree.
- Detail level: structural placeholder prepared for a later deep pass.

## Source Metadata
- Source path: `src/app/qml/view/panels/NavigationBarLayout.qml`
- Source kind: QML view/component
- File name: `NavigationBarLayout.qml`
- Approximate line count: 223

## QML Surface Snapshot
- Root type: `Rectangle`

### Object IDs
- `navigationBar`
- `navigationBarSurface`
- `navigationBarContents`
- `propertiesBar`
- `applicationBarLoader`
- `applicationViewBarComponent`
- `applicationEditBarComponent`
- `applicationControlBarComponent`

### Required Properties
- None detected during scaffold generation.

### Signals
- `compactAddFolderRequested`
- `compactLeadingActionRequested`
- `agendaRequested`
- `toggleDetailPanelRequested`
- `toggleSidebarRequested`
- `viewHookRequested`

## Recent Updates
- Added `pragma ComponentBehavior: Bound` so nested mode `Component` branches can safely reference
  `navigationBar` id members without unqualified-scope warnings.
- Compact mobile right-group contract keeps two independent actions:
  one `nodesnewFolder` add-folder button and one mode-specific context-menu button loaded from the
  active View/Edit/Control application bar component.
- `handleApplicationBarViewHook(...)` now maps `agenda` reasons into `agendaRequested()` so both
  View/Edit menus and the calendar icon row can open the Agenda route through shared wiring.

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
