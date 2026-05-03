# `src/app/qml/Main.qml`

## Role
`Main.qml` is the root LVRS shell. It owns route composition, adaptive layout selection, top-level geometry policy, and direct access to the C++ runtime objects published by startup.

## Root Responsibilities
- Instantiate the `LV.ApplicationWindow`.
- Publish explicit `LV.Theme.gapNone` `topPadding/rightPadding/bottomPadding/leftPadding` properties so the LVRS
  `ApplicationWindow` compatibility bindings have stable geometry inputs on iOS.
- Define root sizing, panel widths, and adaptive layout defaults.
- Resolve runtime controllers from C++ context objects populated by `WhatSonQmlContextBinder`.
- Host the root `windowInteractions` helper object for shortcuts and render-quality policy.
- Keep the desktop workspace shell mounted in `Main.qml` while ordinary desktop onboarding is presented through a
  separate `Onboarding.qml` subwindow.
- Route Android onboarding through the LVRS page stack, while iOS keeps its inline onboarding presentation inside the
  workspace page itself to avoid an extra page-stack flip.
- Disable iOS safe-area/windowing delegation and force LVRS full-window mobile coverage so the app
  content occupies the entire screen while still pinning the render tier to `LowTier`.
- Expose the global `StandardKey.New` note-creation shortcut only on desktop platforms.
- Lazy-load the macOS native menu bar and hand it the root window plus the resources import Controller.
- Forward `resourcesImportController` into both desktop `BodyLayout` and mobile `MobileHierarchyPage` so note-editor
  drag/drop imports and menu-driven imports share the same packaging/runtime-refresh backend.
- The root now resolves that import controller through a dedicated `rootResourcesImportController`
  alias before forwarding it into child components. This avoids QML self-binding loops on child
  properties also named `resourcesImportController`.
- Forward `editorViewModeController` into both desktop `BodyLayout` and mobile `MobileHierarchyPage` so navigation-bar
  editor mode selection (`Plain/Page/Print/Web/Presentation`) drives the same content-surface render policy on every
  platform branch.
- Mobile `MobileHierarchyPage` compact chrome now receives a dedicated root `mobileControlSurfaceColor`
  (`LV.Theme.panelBackground10`) for its control surface, matching the Figma mobile navigation/status surface color
  instead of inheriting the lighter root canvas tone.
- Own Agenda/day/week/month/year overlay visibility flags and keep them mutually exclusive across desktop and mobile
  route handlers.
- Accept `monthCalendarOverlayOpenRequested` from both desktop and mobile content shells so a year-calendar selection
  can swap overlays from year view to month view at the root owner without duplicating visibility state below.
- Reset navigation-opened Agenda/day/week/month/year overlays back to today's date context at the root owner, so a
  reused long-lived calendar Controller does not reopen on a stale historical cursor.

## Runtime Object Access
The important architectural work in this file is not the layout math. It is the explicit context-object hand-off.

`WhatSonQmlContextBinder` publishes stable root objects before `Main.qml` loads, including:
- `libraryHierarchyController`
- `libraryNoteMutationController`
- `navigationModeController`
- `sidebarHierarchyController`
- `noteActiveState`
- `detailPanelController`

`Main.qml` resolves those objects through local root aliases and forwards only the required objects into child views.
There is no QML-side runtime registry or view-model layer in this path.

The root forwards the global `noteActiveState` object into both desktop `BodyLayout` and mobile
`MobileHierarchyPage`, so note activation is tracked by one C++ object instead of each layout independently deriving
the active note-list model. Those branches forward the same object into the editor host, where the visible editor
session is attached back to `noteActiveState` for immediate active-note session synchronization.

## Layout Composition
The file keeps both desktop and mobile layout branches alive.
- Desktop uses the status bar, navigation bar, sidebar, list, content, and detail panel composition.
- Mobile uses routed workspace pages and a scaffold tuned for compact navigation.
- Root-owned sidebar/right-panel minimum and preferred widths plus hierarchy toolbar inset/spacing now route through
  named `LV.Theme` token compositions instead of shell-local pixel literals.
- Ordinary desktop startup can reopen a dedicated `WindowView.Onboarding` subwindow, while Android still uses the
  embedded `/onboarding` route inside `Main.qml`.
- Android embedded startup still relies on the LVRS route stack directly: `Component.onCompleted` seeds the startup
  route, `routeSyncRequested(...)` applies controller-directed transitions, and failure handling stays inside
  `OnboardingRouteBootstrapController` without any extra page-host watchdog or fallback overlay in `Main.qml`.
- iOS keeps the LVRS page stack pinned to `/` and switches the workspace page loader between
  `IosInlineOnboardingSequence.qml` and the real workspace shell. The same `OnboardingRouteBootstrapController` still
  owns transition intent, and iOS completes that intent without a `/onboarding -> /` page-stack flip.
- Desktop `showOnboardingWindow()` now raises the dedicated onboarding subwindow unless the current adaptive/mobile
  presentation is explicitly using an embedded onboarding route.
- `mobileMainLayoutComponent`, `desktopMainLayoutComponent`, and the desktop `WindowView.Onboarding` subwindow must all
  remain sibling nodes inside the same root `LV.ApplicationWindow`; moving the mobile component outside that root
  breaks QML component parsing before the desktop layout branch can load.
- The embedded onboarding/workspace route pages intentionally keep their root `Item` free of `anchors.fill`.
  LVRS route hosting is backed by a `StackView`, and stack-managed page geometry must not compete with page-root
  anchors during transitions on iOS.
- The root `LV.ApplicationWindow` now also exposes explicit `LV.Theme.gapNone` padding properties so LVRS does not emit
  `topPadding/rightPadding/bottomPadding/leftPadding` binding warnings while bootstrapping the iOS shell.
- Desktop panel transparency uses `LV.Theme.accentTransparent` instead of a direct transparent color literal.
- iOS now uses LVRS full-window mobile coverage instead of UIKit safe-area delegation. This keeps
  the application content in true edge-to-edge mode without app-level safe-area color overrides.

## Important Signals and Hooks
- `viewHookRequested` is the root forwarding signal used by nested components that want to bubble interaction intent upward.
- The root `Shortcut` for `StandardKey.New` is intentionally gated behind `isDesktopPlatform` so mobile route changes and
  panel navigation cannot fall into global note creation.
- Desktop `BodyLayout.noteActivated(...)` now clears all Agenda/calendar visibility flags in `Main.qml`, so explicit note
  activation always restores the editor surface even when calendar mode was previously opened.
- Desktop `BodyLayout` and mobile `MobileHierarchyPage` now also forward `monthCalendarOverlayOpenRequested`, which
  closes agenda/day/week/year overlays and shows the month overlay in one root-owned transition.
- Root calendar open helpers now distinguish two entry paths:
  - navigation-bar open actions reset the relevant calendar cursor to today before showing the overlay,
  - `monthCalendarOverlayOpenRequested` preserves the requested year/month/date coming from `YearCalendarPage.qml`
    instead of overwriting it with today's month.
- `Component.onCompleted` performs initial layout stabilization and route/debug policy setup.
- Embedded onboarding route updates now flow only through `applyRequestedRoute(...)` and the onboarding controller's
  `routeSyncRequested(...)` signal; `Main.qml` no longer owns a startup watchdog timer or a fallback overlay.
- `Component.onDestruction` leaves runtime-object lifetime with the C++ composition root.

## Tests

- Automated test files are not currently present in this repository.
- Regression checklist:
  - desktop navigation-bar `Agenda/Day/Week/Month/Year` actions must reopen their overlays on today's date context
    even after the user previously navigated to another date
  - mobile navigation-bar calendar actions must apply the same today-reset behavior
  - mobile compact navigation/status chrome must keep the shared `panelBackground10` control surface tone from the
    Figma mobile shell and must not regress back to the root canvas color
  - a year-calendar month/day tap that emits `monthCalendarOverlayOpenRequested` must still open the requested
    month/date instead of being reset back to today's month
  - embedded startup onboarding must not reintroduce a startup watchdog timer, recovery helper, or fallback overlay in
    `Main.qml`
- iOS startup without a successfully loaded hub must keep the LVRS route pinned to `/` while still presenting the inline
  onboarding sequence inside the same root window
- desktop startup without a successfully loaded hub must reopen the independent onboarding window instead of switching the main
  page stack to `/onboarding`

## Practical Reading
Read this file with:
- `src/app/main.cpp` for object lifetime and context-property creation.
- `src/app/qml/MainWindowInteractionController.qml` for shortcut behavior.
- `src/app/qml/view/panels/HierarchySidebarLayout.qml` for active hierarchy binding into the sidebar.
