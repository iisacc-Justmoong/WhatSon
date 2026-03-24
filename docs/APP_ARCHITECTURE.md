# App Architecture

## Desktop Workspace
The root `ApplicationWindow` `panelBackground01` canvas remains the only broad desktop background surface.

Desktop and shared LVRS control surfaces stay on the lower-luminance alias ladder (`windowAlt -> panelBackground01`,
`subSurface -> panelBackground02`, `surfaceSolid -> panelBackground03`, `surfaceAlt -> panelBackground04`) so the app
does not render a brighter shell than the Figma `ApplicationWindow` reference.

The editor theme contract now keeps the broad desktop editor surfaces transparent, while line-number colors stay on the
dedicated `#4E5157` / `#9DA0A8` contrast pair instead of inheriting the body text tone.

## Mobile Shell
`MobilePageScaffold.qml` is the persistent mobile shell wrapper. It keeps the compact navigation bar and compact status bar mounted while the routed body swaps between hierarchy, note-list, and editor content.

The compact navigation surface stays `24px` high on `panelBackground10`, and the hierarchy route keeps `gap2` VStack spacing so the mobile shell preserves the Figma rhythm instead of introducing a second shell layout.

## Routed Workspace
`MobileHierarchyPage.qml` owns the mobile `LV.PageRouter` stack and mounts the hierarchy page, note-list body, and editor body as routed children.

The same file suppresses the compact leading action on the note-list body and editor body, keeps the shared compact `settings` button exclusive to the hierarchy route, drives interactive back through `LV.PageTransitionController`, and begins that transition from a left-edge touch `DragHandler`.

Editor-pop repair prefers the actually rendered note-list body before rebuilding a canonical stack, so returning from a note does not fall through to the generic All Library state.

## Hierarchy Contract
The mobile shell does not fork `Hierarchy.editable`; it forwards the shared reorder capability into the mounted hierarchy surface instead of creating a mobile-only hierarchy interaction model.

Library system buckets now emit `draggable`, `dragAllowed`, `movable`, and `dragLocked`, and the hierarchy row baseline still resolves to a `20px` LVRS item height.

`MobileHierarchyPage.qml` disables `usePlatformSafeMargin`, keeps the compact navigation bar and compact status bar mounted, and keeps the hierarchy column on the same mobile canvas instead of painting an isolated nested panel surface.

## Control Surfaces
The compact control menu anchors from the trigger's bottom-right point. On the mobile hierarchy/control route, that trigger uses the `toolwindowtodo` glyph plus the built-in LVRS chevron instead of the old project-structure icon, and the trigger keeps the Figma `2 / 4 / 2 / 2` padding contract.

Action-only control entries disable the default LVRS shortcut placeholder column so icon-only mobile actions keep their full available label width.
