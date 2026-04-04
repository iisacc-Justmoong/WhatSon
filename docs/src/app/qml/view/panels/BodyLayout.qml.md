# `src/app/qml/view/panels/BodyLayout.qml`

## Status
- Documentation phase: scaffold generated from the live source tree.
- Detail level: structural placeholder prepared for a later deep pass.

## Source Metadata
- Source path: `src/app/qml/view/panels/BodyLayout.qml`
- Source kind: QML view/component
- File name: `BodyLayout.qml`
- Approximate line count: 233

## QML Surface Snapshot
- Root type: `Item`

### Object IDs
- `hStack`
- `sideBar`
- `sideBarSplitter`
- `listBar`
- `listSplitter`
- `contentsView`
- `rightSplitter`
- `rightPanel`

### Required Properties
- `sidebarHierarchyViewModel`

### Signals
- `drawerHeightDragRequested`
- `listViewWidthDragRequested`
- `noteActivated`
- `rightPanelWidthDragRequested`
- `sidebarWidthDragRequested`
- `viewHookRequested`
- `agendaOverlayDismissRequested`
- `dayCalendarOverlayDismissRequested`
- `monthCalendarOverlayDismissRequested`
- `weekCalendarOverlayDismissRequested`
- `yearCalendarOverlayDismissRequested`

## Route Bridging Notes
- `ListBarLayout.noteActivated(...)` is re-emitted as `BodyLayout.noteActivated(...)`.
- Root containers (for example `Main.qml`) can consume this bridge signal to reset calendar-route visibility and
  guarantee editor resurfacing when a note is explicitly activated.
- Agenda/day/week/month/year overlay visibility and viewmodel handles are forwarded to `ContentViewLayout` so panel-level
  routing remains centralized in the root shell.
- `resourcesImportViewModel` is accepted on the desktop shell and forwarded to `ContentViewLayout`, so file drops on
  the note editor reuse the same `.wsresource` import path as menu-based imports.
- `editorViewModeViewModel` is accepted on the desktop shell and forwarded to `ContentViewLayout`, so editor render
  modes (`Plain/Page/Print/Web/Presentation`) stay consistent between navigation controls and the content surface.
- `isMobilePlatform` is forwarded from the root LVRS window into `ContentViewLayout`, even on the desktop shell, so
  nested editor components consume the canonical LVRS platform detector instead of recomputing mobile mode locally.

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
