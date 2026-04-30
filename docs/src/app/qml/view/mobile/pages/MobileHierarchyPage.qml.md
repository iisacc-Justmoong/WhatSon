# `src/app/qml/view/mobile/pages/MobileHierarchyPage.qml`

## Role
`MobileHierarchyPage.qml` is the routed mobile workspace shell for hierarchy browsing, folder-scoped note lists, note
editing, and the detail page.

It does not own the domain data itself. Its job is to keep the mobile `LV.PageRouter` stack aligned with the already-bound hierarchy and note-list controllers that come from the desktop-style application root.

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
- Treat mobile back navigation as page dismissal instead of a raw router pop, so `/mobile/detail` dismisses to the
  editor, `/mobile/editor` dismisses to the current note list, and `/mobile/note-list` dismisses to the hierarchy
  root without letting a transient hierarchy pop clear the active folder selection first.
- Own the mobile panel hook directly through the `mobile.MobileHierarchyPage` panel key instead of routing that hook through a wrapper component, while keeping note creation on the dedicated `windowInteractions` shortcut path so generic route hooks never create files.
- Resolve the effective list-deletion contract from the active hierarchy first, then fall back to
  the shared library-note mutation controller (or the global LV controller registry) when the active
  hierarchy does not own deletion itself.
- Snapshot the active toolbar index and active hierarchy content controller as one binding bundle whenever
  `SidebarHierarchyController.activeBindingsChanged()` fires, so mobile route decisions do not momentarily mix library
  state with resources-state chrome during hierarchy switches.
- The active note-list model is read directly from `SidebarHierarchyController.activeNoteListModel`.
  The mobile route shell does not mount a second local note-list bridge, so the hierarchy index, active hierarchy
  controller, and note/resource list model all come from the same sidebar binding authority.

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
The compact editor-view selector is also bound to `editorPageActive` (`compactEditorViewVisible`), so
`/mobile/editor` renders the dual-combo navigation row (`NavigationModeBar` + `NavigationEditorViewBar`)
matching Figma node `174:5689`, while hierarchy and note-list routes keep their previous compact chrome.
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

`ContentViewLayout.qml` can now emit `monthCalendarOverlayOpenRequested()` from inside the mobile editor route when the
user taps a month or day in `YearCalendarPage.qml`. `MobileHierarchyPage.qml` re-emits that signal so the app root can
swap the visible overlay from year to month while keeping the overlay state owned above the router shell.
The same content route also forwards `sidebarHierarchyController` into `ContentViewLayout.qml`, so tapping a projected
calendar note on mobile can switch the active hierarchy back to Library, select the note, dismiss the calendar
overlay, and leave the user on the editor route with that note visible.

## Mobile Detail Page
The compact navigation's right-edge `DetailPanelControlButton` now routes into `/mobile/detail`.

The detail-page behavior is:
- opened only from `/mobile/editor`
- rendered through the existing `DetailPanelLayout.qml`
- mounted as a normal routed page body, so it participates in the same back-swipe/page-stack model as note list and editor
- stretched to the full routed-body width provided by `MobilePageScaffold.qml`, so the detail surface reaches the mobile safe-area bounds instead of using a centered fixed-width card
- no longer exposed as a compact context-menu item

The detail route must not reintroduce its own width clamp. `MobilePageScaffold.qml` already constrains the routed body
to the safe-area-aware mobile content width, so `/mobile/detail` should simply fill that body.

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

The explicit mobile back action now avoids that transient state entirely. `dismissCurrentPage()` first asks
`MobileHierarchyNavigationCoordinator::dismissPagePlan(...)` for the canonical dismiss target and then routes directly
to that target instead of calling `router.back()`. This keeps editor dismissal on the stable
`hierarchy -> note-list -> editor` model even when the live router stack has drifted.

## Detail Pop Repair
Interactive back navigation from `/mobile/detail` is now repaired with the same explicit post-commit verification used by the editor route.

- `handleCommittedRouteTransition(...)` now distinguishes `fromPath === /mobile/detail` from `fromPath === /mobile/editor`.
- `verifyCommittedDetailPopState(...)` waits for the routed body to settle and accepts only `/mobile/editor` as the valid restore target.
- If the router or rendered body drifts to hierarchy or another non-editor state after a committed detail-page pop, the page rebuilds the canonical `hierarchy -> note-list -> editor` stack instead of leaving the user on the wrong destination.

This preserves the user expectation that dismissing the mobile detail page returns to the current note editor, not to the hierarchy root.

The page re-runs `syncRouteSelectionState()` from both router-path changes and
`resolvedBodyRoutePath` changes. That dual trigger is intentional because the router can update
before the rendered body catches up, and the hierarchy-selection clear must wait until both layers
agree that the user is truly back on the hierarchy root.

## Back Swipe
The left-edge gesture is implemented with a local `DragHandler` instead of a global gesture listener.

The flow is:
1. `beginBackSwipeGesture(...)` validates the edge hit and begins `LV.PageTransitionController`.
2. `updateBackSwipeGesture(...)` feeds progress and cancels when the motion turns vertical.
3. `finishBackSwipeGesture(...)` cancels the gesture when the controller rejects the commit and otherwise routes
   through the same dismiss helper used by the explicit back action.

This keeps mobile back navigation local to the page and avoids stealing editor text-selection gestures from the content view.

## Collaborators
- `MobilePageScaffold.qml`: owns the shared compact navigation and status chrome.
- `MobileNoteCreationCoordinator.qml`: owns create-note dispatch plus pending-note promotion into the editor route.
- `HierarchySidebarLayout.qml`: renders the hierarchy route body.
- `ListBarLayout.qml`: renders the folder-scoped note list route body.
- The routed `ListBarLayout.qml` now receives `resolvedNoteDeletionController`, so hardware-keyboard
  `Delete` / `Backspace` in the mobile note list can remove resource packages when the resources
  hierarchy owns the active list.
- `ContentViewLayout.qml`: renders the editor route body and now mounts the unified `ContentsDisplayView.qml` host in
  mobile mode.
- The mobile editor route still no longer relies on ad hoc desktop suppression flags:
  - gutter is removed by the shared host's mobile mode policy
  - editor font size now stays aligned with the desktop `12px` baseline
- The route forwards the LVRS window `isMobilePlatform` state into `ContentViewLayout`, so the unified host's mode
  policy uses the canonical platform detector instead of responsive-width guesses.
- `resourcesImportController`: forwarded into `ContentViewLayout` so editor drops on mobile can package files and emit
  `<resource ...>` links through the same import pipeline.
- `editorViewModeController`: forwarded into `ContentViewLayout` so mobile editor mode selection uses the same
  plain/print/preview rendering policy as desktop.
- `SidebarHierarchyController`: supplies the active hierarchy domain, note-list model, and hierarchy selection.
- The same shared sidebar controller is also forwarded into `ContentViewLayout.qml`, so mobile calendar note taps reuse
  the same library-selection path as desktop.
- `windowInteractions`: routes the dedicated create-note action and resolves the writable note-mutation capability.
- The mobile page now defaults `controlSurfaceColor` to `LV.Theme.panelBackground10`, matching the Figma mobile
  navigation/status surface token while still letting the root override that shared compact chrome color explicitly.

## Known Invariants
- A note-list/editor canonical rebuild must preserve the hierarchy selection before changing the route stack.
- Returning from `/mobile/editor` must prefer the actually displayed note-list body over a stale `currentPath` snapshot.
- Returning from `/mobile/detail` must prefer the actual editor body and must never settle on the hierarchy page as the restored destination.
- Explicit mobile back dismissal must never call a raw `router.back()` from `/mobile/editor`, because the transient
  hierarchy route can clear the active hierarchy selection before note-list restoration runs.
- Mobile hierarchy routing is selection-driven; the routes do not own separate domain state copies.
- Mobile hierarchy routing must refresh its active toolbar/content tuple from one snapshot and forward the resulting
  `activeNoteListModel` into both `ListBarLayout.qml` and `ContentViewLayout.qml`, or transient hierarchy switches can
  leave the routed note list out of sync with the selected sidebar domain.
- Mobile hierarchy routing must also resolve that hierarchy snapshot from the selected hierarchy index itself, not only
  from one previously cached "active" provider object, or a toolbar switch can leave the old domain controller
  mounted.
- The mobile editor page must not reintroduce gutter width/line-number overrides now that mobile gutter policy lives in
  `ContentsDisplayHostModePolicy.qml`.
- The mobile editor page must keep sourcing platform mode from the LVRS window detector, not from viewport width.
- The mobile editor page must not fall back to the desktop editor file for mobile rendering.
- The mobile detail route must fill the routed body width end-to-end; centered fixed-width detail cards are a regression on
  mobile.
- Calendar note taps from the mobile editor route must keep using the shared library selection state instead of
  introducing a mobile-only note activation store.
- Mobile note-list deletion must continue to prefer the active hierarchy's own delete contract when
  one is available, so resource-package deletion does not fall back to library-note deletion.
