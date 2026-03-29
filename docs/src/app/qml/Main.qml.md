# `src/app/qml/Main.qml`

## Role
`Main.qml` is the root LVRS shell. It owns route composition, adaptive layout selection, top-level geometry policy, and QML-side registration of root viewmodels into `LV.ViewModels`.

## Root Responsibilities
- Instantiate the `LV.ApplicationWindow`.
- Define root sizing, panel widths, and adaptive layout defaults.
- Register runtime viewmodels into the LVRS `ViewModels` registry.
- Claim writable ownership for selected interaction surfaces.
- Mount `MainWindowInteractionController` and feed it the objects it needs for shortcuts and render-quality policy.
- Switch between onboarding and workspace routes.
- Recover embedded mobile startup routes when the LVRS page host resolves to an empty path or a missing page item.
- Keep iOS physical-device startup on the conservative LVRS windowing path by delegating windowing/insets back to the
  system and pinning the render tier to `LowTier`.
- Expose the global `StandardKey.New` note-creation shortcut only on desktop platforms.
- Lazy-load the macOS native menu bar and hand it the root window plus the resources import ViewModel.

## ViewModel Ownership
The important architectural work in this file is not the layout math. It is the ownership hand-off.

`registerRootViewModels()` copies context-property objects into `LV.ViewModels` under stable keys such as:
- `libraryHierarchyViewModel`
- `libraryNoteMutationViewModel`
- `navigationModeViewModel`
- `sidebarHierarchyViewModel`
- `detailPanelViewModel`

`bindOwnedViewModel(...)` then claims write ownership for concrete view IDs:
- `windowInteractions.libraryNoteMutation`
- `windowInteractions.navigationMode`
- `windowInteractions.sidebarHierarchy`

This means the root scene stops behaving like a bag of globally mutable QObjects and starts behaving more like explicit LVRS-owned writable surfaces.

## Layout Composition
The file keeps both desktop and mobile layout branches alive.
- Desktop uses the status bar, navigation bar, sidebar, list, content, and detail panel composition.
- Mobile uses routed workspace pages and a scaffold tuned for compact navigation.
- Onboarding can be embedded into the route stack or opened as a separate window depending on platform and adaptive mode.
- Embedded mobile startup now keeps an app-owned watchdog around the routed page host. If the expected onboarding or
  workspace route exists in controller state but the active LVRS router has no current page item, `Main.qml` forces a
  `setRoot(...)` rebuild on the expected route and shows a temporary fallback surface instead of leaving a black frame.
- iOS now opts out of the LVRS full-window mobile coverage override. That reduces first-frame Metal swapchain churn on
  physical devices by letting UIKit own fullscreen/window transitions while the app uses the lower-cost LVRS
  `LowTier` render preset.

## Important Signals and Hooks
- `viewHookRequested` is the root forwarding signal used by nested components that want to bubble interaction intent upward.
- The root `Shortcut` for `StandardKey.New` is intentionally gated behind `isDesktopPlatform` so mobile route changes and
  panel navigation cannot fall into global note creation.
- `Component.onCompleted` performs registry registration, ownership binding, and initial layout stabilization.
- `syncEmbeddedRouteWatchdog(...)` and `recoverEmbeddedRouteHost(...)` provide a first-frame safety net for iOS/mobile
  startup so a missing routed page host turns into a controlled route rebuild instead of a blank screen.
- `Component.onDestruction` releases owned view bindings so the registry does not retain stale writable handles.

## Practical Reading
Read this file with:
- `src/app/main.cpp` for object lifetime and context-property creation.
- `src/app/qml/MainWindowInteractionController.qml` for shortcut behavior.
- `src/app/qml/view/panels/HierarchySidebarLayout.qml` for active hierarchy binding into the sidebar.
