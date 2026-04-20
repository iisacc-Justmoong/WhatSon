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
- Forwards compact detail-page state into `NavigationBarLayout` and re-emits `toggleDetailPanelRequested()` back to
  `MobileHierarchyPage`, where that signal now pushes the dedicated `/mobile/detail` route instead of opening an
  overlay or context-menu action.
- `bodyRouter` is the safe-area-bounded mobile body surface. Routed pages such as `/mobile/detail` are expected to fill
  that width directly instead of applying their own centered fixed-width clamp.
- Forwards `compactEditorViewVisible` into `NavigationBarLayout` so the compact mobile editor route can replace the
  hierarchy-only settings affordance with the editor `View mode` combo (`NavigationEditorViewBar`) without introducing
  route-local navigation bar forks.
- The scaffold now defaults `controlSurfaceColor` to `LV.Theme.panelBackground10`, which matches the shared Figma
  mobile navigation/status surface fill used by both the compact top bar and the compact status search field.

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
