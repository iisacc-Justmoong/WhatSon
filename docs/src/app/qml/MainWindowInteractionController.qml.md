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

Only if no owned viewmodel is available does it fall back to the directly injected bootstrap object. This keeps older composition paths alive without giving up the new LVRS ownership model.

## Key Functions
- `createNoteFromShortcut()`: routes note creation through the panel registry if available, otherwise through the owned note-mutation or hierarchy viewmodel.
- `cycleNavigationModeFromShortcut()`: advances navigation mode unless a text input currently holds focus.
- `handleResizeForRenderQuality()` and `finalizeResizeRenderQualityPolicy()`: temporarily suspend dynamic resolution during active resize to avoid unstable render behavior.
- `clearActiveFocus(...)` and `shouldRetainFocusForUiHit(...)`: define when a click should clear editing focus and when it should not.

## Why It Exists
Without this object, `Main.qml` would have to mix root layout, route setup, keyboard shortcuts, focus heuristics, and render-policy bookkeeping in one file. This object keeps those responsibilities separate while still staying QML-local.
