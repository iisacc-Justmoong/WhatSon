# `src/app/qml/view/mobile/MobilePageScaffold.qml`

## Status
- Documentation phase: scaffold generated from the live source tree.
- Detail level: structural placeholder prepared for a later deep pass.

## Source Metadata
- Source path: `src/app/qml/view/mobile/MobilePageScaffold.qml`
- Source kind: QML view/component
- File name: `MobilePageScaffold.qml`
- Approximate line count: 94

## QML Surface Snapshot
- Root type: `Rectangle`

### Object IDs
- `mobilePageScaffold`
- `bodyRouter`

### Required Properties
- None detected during scaffold generation.

### Signals
- `compactAddFolderRequested`
- `compactLeadingActionRequested`
- `createNoteRequested`
- `agendaRequested`
- `dayCalendarRequested`
- `weekCalendarRequested`
- `monthCalendarRequested`
- `yearCalendarRequested`
- `statusSearchSubmitted`
- `statusSearchTextEdited`
- `toggleDetailPanelRequested`
- `viewHookRequested`

## Recent Updates
- Forwards `NavigationBarLayout` Agenda/day/week/month/year hook signals so mobile calendar routing can open overlays on
  the editor surface through `MobileHierarchyPage`.
- Owns a dedicated `bodyHost` wrapper around `LV.PageRouter`, so route bodies can now render an editor-local overlay
  component above the mobile page body without covering the compact navigation or status bar.
- Forwards compact detail-panel state into `NavigationBarLayout` and re-emits
  `toggleDetailPanelRequested()` back to `MobileHierarchyPage`.

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
