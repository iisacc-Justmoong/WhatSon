# `src/app/qml/MainWindowInteractionController.qml`

## Role
This `QtObject` centralizes root-window interaction policy that would otherwise clutter `Main.qml`.

It is not a generic controller for the whole application. It specifically owns root-window concerns:
- shortcut-triggered note creation
- shortcut-triggered navigation mode cycling
- render-quality suspension and resume during resize
- focus clearing heuristics
- layout and route debug reporting

## Ownership-Aware Behavior
The file is notable because it tries writable ownership first.
- For note creation it asks `LV.ViewModels.getForView(libraryNoteMutationViewId)`.
- For navigation mode cycling it asks `LV.ViewModels.getForView(navigationModeViewId)`.
- For sidebar activation before note creation it asks `LV.ViewModels.getForView(sidebarHierarchyViewId)`.

If no owned object is currently mounted for a writable view id, the controller falls back to the stable root
`LV.ViewModels` registry key for that capability. It does not depend on direct bootstrap-object injection anymore, so
the shortcut layer stays aligned with the same ownership contract used by the rest of the scene.

## Key Functions
- `createNoteFromShortcut()`: routes note creation through the panel registry if available, otherwise through the owned note-mutation or hierarchy viewmodel. `Main.qml` must decide when that helper is exposed; the global `StandardKey.New` shortcut is desktop-only, while mobile calls come from explicit UI controls.
- `cycleNavigationModeFromShortcut()`: advances navigation mode unless a text input currently holds focus.
- `resolveOwnedWritableViewModel(...)`: normalizes writable lookup around LVRS view ownership plus the stable registry key for the same capability.
- `resolvePanelViewModel(...)`: resolves panel-scoped hooks without forcing every consumer to know the registry contract.
- `handleResizeForRenderQuality()` and `finalizeResizeRenderQualityPolicy()`: temporarily suspend dynamic resolution during active resize, but only on desktop. Mobile/iOS keeps the guard disabled so startup geometry churn does not force extra Metal swapchain rebuilds.
- `applyRenderQualityPolicy()`: logs the effective guard mode together with the root window's device-tier and mobile-windowing policy so iOS/Xcode crash reports can distinguish render-tier issues from fullscreen coverage churn.
- `clearActiveFocus(...)` and `shouldRetainFocusForUiHit(...)`: define when a click should clear editing focus and when it should not.

## Why It Exists
Without this object, `Main.qml` would have to mix root layout, route setup, keyboard shortcuts, focus heuristics, and render-policy bookkeeping in one file. This object keeps those responsibilities separate while still staying QML-local.
