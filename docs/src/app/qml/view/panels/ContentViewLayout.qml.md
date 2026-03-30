# `src/app/qml/view/panels/ContentViewLayout.qml`

## Status
- Documentation phase: baseline updated from the live source tree.
- Detail level: focused on calendar overlay routing and ownership.

## Source Metadata
- Source path: `src/app/qml/view/panels/ContentViewLayout.qml`
- Source kind: QML view/component
- File name: `ContentViewLayout.qml`
- Approximate line count: 200+

## QML Surface Snapshot
- Root type: `Item`

### Object IDs
- `contentViewLayout`

### Required Properties
- None detected during scaffold generation.

### Signals
- `drawerHeightDragRequested`
- `editorTextEdited`
- `dayCalendarOverlayCloseRequested`
- `monthCalendarOverlayCloseRequested`
- `weekCalendarOverlayCloseRequested`
- `viewHookRequested`
- `yearCalendarOverlayCloseRequested`

## Overlay Contract
- Imports `../calendar` as `CalendarView` and mounts day/week/month/year pages through one overlay `Loader`.
- Uses per-mode visibility/viewmodel pairs:
  - `dayCalendarOverlayVisible` / `dayCalendarViewModel`
  - `weekCalendarOverlayVisible` / `weekCalendarViewModel`
  - `monthCalendarOverlayVisible` / `monthCalendarViewModel`
  - `yearCalendarOverlayVisible` / `yearCalendarViewModel`
- Resolves the active page by strict priority:
  - day first
  - then week
  - then month
  - then year
- Exposes dedicated dismiss signals for each mode:
  - `dayCalendarOverlayCloseRequested`
  - `weekCalendarOverlayCloseRequested`
  - `monthCalendarOverlayCloseRequested`
  - `yearCalendarOverlayCloseRequested`
- The overlay background click and close button both dispatch through
  `requestActiveCalendarOverlayClose()` so parent containers (`BodyLayout`, `MobileHierarchyPage`) keep visibility
  ownership and can enforce mutually exclusive mode flags.

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
