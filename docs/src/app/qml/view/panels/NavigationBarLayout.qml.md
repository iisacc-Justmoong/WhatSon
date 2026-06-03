# `src/app/qml/view/panels/NavigationBarLayout.qml`

## Status
- Documentation phase: scaffold generated from the live source tree.
- Detail level: structural placeholder prepared for a later deep pass.

## Source Metadata
- Source path: `src/app/qml/view/panels/NavigationBarLayout.qml`
- Source kind: QML view/component
- File name: `NavigationBarLayout.qml`
- Approximate line count: 262

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
- `dayCalendarRequested`
- `monthCalendarRequested`
- `toggleDetailPanelRequested`
- `toggleSidebarRequested`
- `viewHookRequested`
- `weekCalendarRequested`
- `yearCalendarRequested`

## Recent Updates
- Added `pragma ComponentBehavior: Bound` so nested mode `Component` branches can safely reference
  `navigationBar` id members without unqualified-scope warnings.
  one `nodesnewFolder` add-folder button and one mode-specific application-bar slot loaded from the
  active View/Edit/Control application bar component.
  the active View/Edit/Control application bar component. This lets the active content route show a dedicated
  right-edge detail-page icon button without leaking that affordance into hierarchy or note-list routes, and without
  duplicating that action inside the compact context menus.
- The editor view-mode selector is removed. Desktop and compact navigation must not forward editor view-mode
  controllers or mount editor-specific view chrome.
  `panelBackground10` (`#343536`) for that pill background instead of leaving it transparent or inheriting the page
  canvas tone.
- Calendar hook reasons emitted by the View/Edit application bars are normalized in `handleApplicationBarViewHook(...)`.
  Reasons containing `daily-calendar`, `weekly-calendar`, `monthly-calendar`, or `yearly-calendar` raise the matching
  calendar request signal so `Main.qml` can open the corresponding content overlay.

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
