# `src/app/qml/view/panels/ContentViewLayout.qml`

## Status
- Documentation phase: baseline updated from the live source tree.
- Detail level: focused on calendar content-surface routing and ownership.

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
- `agendaOverlayCloseRequested`
- `monthCalendarOverlayCloseRequested`
- `weekCalendarOverlayCloseRequested`
- `viewHookRequested`
- `yearCalendarOverlayCloseRequested`

## Content Surface Contract
- Imports `../calendar` as `CalendarView` and mounts agenda/day/week/month/year pages through one content-surface `Loader`.
- Uses a `StackLayout` (`currentIndex: activeSurfaceIndex`) so the editor surface and calendar surface are switched by
  route state, not by overlay stacking.
- Both stack children (`editorContentSurface`, `calendarContentSurface`) explicitly apply
  `Layout.fillWidth: true` and `Layout.fillHeight: true` so calendar pages occupy the full `ContentsView` slot.
- Uses per-mode visibility/viewmodel pairs:
  - `agendaOverlayVisible` / `agendaViewModel`
  - `dayCalendarOverlayVisible` / `dayCalendarViewModel`
  - `weekCalendarOverlayVisible` / `weekCalendarViewModel`
  - `monthCalendarOverlayVisible` / `monthCalendarViewModel`
  - `yearCalendarOverlayVisible` / `yearCalendarViewModel`
- Derives `activeSurfaceIndex` from `calendarOverlayVisible`:
  - `0` => editor (`ContentsDisplayView`)
  - `1` => calendar content surface
- Resolves the active page by strict priority:
  - agenda
  - then day
  - then week
  - then month
  - then year
- Exposes dedicated dismiss signals for each mode:
  - `agendaOverlayCloseRequested`
  - `dayCalendarOverlayCloseRequested`
  - `weekCalendarOverlayCloseRequested`
  - `monthCalendarOverlayCloseRequested`
  - `yearCalendarOverlayCloseRequested`
- The editor (`ContentsDisplayView`) and calendar content surface are mutually exclusive at the same anchor slot.
- The active note-list model's `currentNoteIdChanged` signal dispatches through
  `requestActiveCalendarOverlayClose()` so parent containers (`BodyLayout`, `MobileHierarchyPage`) keep visibility
  ownership and calendar state returns to the editor when a different note is selected.
- The active note-list model's `currentIndexChanged` path also dispatches through
  `requestActiveCalendarOverlayClose()` so route recovery does not depend only on note-id churn.
- The composed `ContentsDisplayView` binding path now keeps explicit root-id qualification for
  ownership-sensitive bindings (`anchors.fill`, `enabled`) to preserve deterministic panel-level
  scope resolution.
- `resourcesImportViewModel` is now part of the wrapper contract and is forwarded into
  `ContentsDisplayView`, enabling editor-side drag/drop packaging through the shared resource import pipeline.

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
