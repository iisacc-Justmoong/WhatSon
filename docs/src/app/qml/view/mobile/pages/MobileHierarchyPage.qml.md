# `src/app/qml/view/mobile/pages/MobileHierarchyPage.qml`

## Role
`MobileHierarchyPage.qml` is the routed mobile workspace shell for hierarchy browsing, folder-scoped note lists, note
editing, and the detail page.

It does not own the domain data itself. Its job is to keep the mobile `LV.PageRouter` stack aligned with the already-bound hierarchy and note-list viewmodels that come from the desktop-style application root.

## Primary Responsibilities
- Mount the four mobile body routes: hierarchy, note list, editor, and detail.
- Mount the mobile detail route on the same `LV.PageRouter`, so note metadata is opened as a first-class page instead
  of an editor overlay or context-menu action.
- Preserve the selected hierarchy folder while the user moves between note list and editor pages.
- Drive left-edge back-swipe gestures through `LV.PageTransitionController`.
- Delegate newly created library-note promotion into a dedicated coordinator once the shared models are ready.
- Keep compact navigation chrome route-aware so hierarchy-only controls do not leak into note-list or editor pages.
- Keep the mobile detail route-aware: the compact navigation only exposes the detail affordance on `/mobile/editor`,
  and that affordance pushes a real routed page instead of toggling overlay chrome.
- Own the mobile panel hook directly through the `mobile.MobileHierarchyPage` panel key instead of routing that hook through a wrapper component, while keeping note creation on the dedicated `windowInteractions` shortcut path so generic route hooks never create files.

## Routing Model
The file defines four route constants:
- `/mobile/hierarchy`
- `/mobile/note-list`
- `/mobile/editor`
- `/mobile/detail`

`mobileBodyRoutes` maps these paths to `HierarchySidebarLayout`, `ListBarLayout`, `ContentViewLayout`, and a dedicated
detail-page wrapper around `DetailPanelLayout`.

The important rule is that the route stack is only canonicalized when the visible body and the router state have genuinely diverged. The helper `displayedBodyRoutePath()` reads the currently mounted scaffold body first and only falls back to `activePageRouter.currentPath` when the body has not resolved yet.

`hierarchyPageActive`, `noteListPageActive`, and `editorPageActive` are all derived from that resolved body path. The compact `settings` affordance is bound to `hierarchyPageActive`, which means the shared navigation bar only shows that button on `/mobile/hierarchy`.
The compact detail-page affordance is bound to `editorPageActive`, which means the new right-edge
`columnIndex` button only appears on `/mobile/editor` and pushes `/mobile/detail` instead of opening a context menu or overlay.
That compact button emits the explicit hook reason `open-detail-page`, so the route push is no longer described as a
collapse/expand action.

`requestOpenNoteList(...)` is subscribed only to `HierarchySidebarLayout.hierarchyItemActivated(...)`.
Chevron-only expansion in `SidebarHierarchyView.qml` is intentionally suppressed before that signal is
re-emitted, so tapping a folder chevron on mobile does not push `/mobile/note-list`.

Before opening `/mobile/editor`, `requestOpenEditor(...)` now calls
`dismissCalendarOverlaysForEditorActivation()` and emits the per-mode dismiss signals. This ensures calendar state is
cleared in the parent owner and prevents the editor route from remaining hidden behind stale calendar visibility flags.
The dismiss set includes `agendaOverlayDismissRequested()` in addition to day/week/month/year routes.

## Mobile Calendar Routing
Calendar pages are rendered by `ContentViewLayout`, which is mounted on the `/mobile/editor` route.

To keep calendar open behavior deterministic on mobile, the scaffold calendar hooks now route through:
- `ensureCalendarSurfaceVisible()`
- `requestOpenAgenda()`
- `requestOpenDayCalendar()`
- `requestOpenWeekCalendar()`
- `requestOpenMonthCalendar()`
- `requestOpenYearCalendar()`

`ensureCalendarSurfaceVisible()` canonicalizes the stack to the editor route when needed, then the corresponding
calendar request signal is emitted. This prevents no-op calendar taps while the user is still on hierarchy or note-list
routes.
Opening any calendar overlay now canonicalizes back to the editor route when needed, so calendar pages do not stack on
top of `/mobile/detail`.

## Mobile Detail Page
The compact navigation's right-edge `DetailPanelControlButton` now routes into `/mobile/detail`.

The detail-page behavior is:
- opened only from `/mobile/editor`
- rendered through the existing `DetailPanelLayout.qml`
- mounted as a normal routed page body, so it participates in the same back-swipe/page-stack model as note list and editor
- no longer exposed as a compact context-menu item

## Selection Preservation
`preservedNoteListSelectionIndex` caches the active hierarchy selection that produced the current note list.

This is used by:
- `requestOpenNoteList(...)`
- `requestOpenEditor(...)`
- `routeToCanonicalNoteList(...)`
- `routeToCanonicalEditor(...)`

The cache prevents a canonical rebuild from collapsing back to the implicit "All Library" list when the user actually came from a folder-specific list.

`syncRouteSelectionState()` now clears the active hierarchy selection only when the routed body and
the router have both settled on `/mobile/hierarchy`, and only when the router stack depth is `<= 1`.
This prevents transient editor-pop states from writing `-1` back into the shared hierarchy
selection while the app is actually returning to a folder-scoped note list.

All selection/session integer parsing in this file now uses an explicit finite-number normalization
helper instead of `Number(value) || -1`. This preserves legitimate `0` values (for example the first
hierarchy item index and touch session id `0`) and prevents accidental fallback to `-1`.

## Editor Pop Repair
Interactive back navigation from the editor can temporarily leave `currentPath` and the rendered body out of sync.

`handleCommittedRouteTransition(...)` and `verifyCommittedEditorPopState(...)` therefore use `displayedBodyRoutePath()` before forcing a canonical note-list rebuild. If the rendered body is already the note-list page, the repair path is skipped. This keeps the previous folder-scoped note list on screen instead of rebuilding the stack back through the generic root state.

The page re-runs `syncRouteSelectionState()` from both router-path changes and
`resolvedBodyRoutePath` changes. That dual trigger is intentional because the router can update
before the rendered body catches up, and the hierarchy-selection clear must wait until both layers
agree that the user is truly back on the hierarchy root.

## Back Swipe
The left-edge gesture is implemented with a local `DragHandler` instead of a global gesture listener.

The flow is:
1. `beginBackSwipeGesture(...)` validates the edge hit and begins `LV.PageTransitionController`.
2. `updateBackSwipeGesture(...)` feeds progress and cancels when the motion turns vertical.
3. `finishBackSwipeGesture(...)` commits or cancels based on controller heuristics.

This keeps mobile back navigation local to the page and avoids stealing editor text-selection gestures from the content view.

## Collaborators
- `MobilePageScaffold.qml`: owns the shared compact navigation and status chrome.
- `MobileNoteCreationCoordinator.qml`: owns create-note dispatch plus pending-note promotion into the editor route.
- `HierarchySidebarLayout.qml`: renders the hierarchy route body.
- `ListBarLayout.qml`: renders the folder-scoped note list route body.
- `ContentViewLayout.qml`: renders the editor route body and now selects `MobileContentsDisplayView.qml` for mobile.
- The mobile editor route no longer relies on desktop editor code plus mobile suppression flags:
  - gutter is removed by the mobile-only editor file itself
  - editor font size is `14px` (`desktop 12px + 2px`)
- The route forwards the LVRS window `isMobilePlatform` state into `ContentViewLayout`, so editor file selection uses
  the canonical platform detector instead of responsive-width guesses.
- `resourcesImportViewModel`: forwarded into `ContentViewLayout` so editor drops on mobile can package files and emit
  `<resource ...>` links through the same import pipeline.
- `editorViewModeViewModel`: forwarded into `ContentViewLayout` so mobile editor mode selection uses the same
  plain/print/preview rendering policy as desktop.
- `SidebarHierarchyViewModel`: supplies the active hierarchy domain, note-list model, and hierarchy selection.
- `windowInteractions`: routes the dedicated create-note action and resolves the writable note-mutation capability.

## Known Invariants
- A note-list/editor canonical rebuild must preserve the hierarchy selection before changing the route stack.
- Returning from `/mobile/editor` must prefer the actually displayed note-list body over a stale `currentPath` snapshot.
- Mobile hierarchy routing is selection-driven; the routes do not own separate domain state copies.
- The mobile editor page must not reintroduce gutter width/line-number overrides now that mobile gutter policy lives in
  `MobileContentsDisplayView.qml`.
- The mobile editor page must keep sourcing platform mode from the LVRS window detector, not from viewport width.
- The mobile editor page must not fall back to the desktop editor file for mobile rendering.
