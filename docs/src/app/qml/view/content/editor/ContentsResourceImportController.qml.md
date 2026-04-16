# `src/app/qml/view/content/editor/ContentsResourceImportController.qml`

## Role
`ContentsResourceImportController.qml` is now the thin editor-side resource import coordinator shared by the desktop
and mobile hosts.

## Responsibilities
- Delegates payload parsing to `ContentsResourceDropPayloadParser.qml`.
- Delegates RAW tag insertion and tag-loss detection to `ContentsResourceTagController.qml`.
- Delegates fallback RichText placeholder/image HTML building to
  `ContentsInlineResourcePresentationController.qml`.
- Delegates guard/programmatic-surface restore state to `ContentsEditorSurfaceGuardController.qml`.
- Delegates duplicate-import prompt state and policy execution to
  `ContentsResourceImportConflictController.qml`.

## Coupling Boundary
- Desktop `ContentsDisplayView.qml` and mobile `MobileContentsDisplayView.qml` now delegate resource import behavior to
  this controller instead of duplicating the same flow in each host.
- The coordinator still exposes the same host-facing API surface, but transient duplicate-import and editor-surface
  guard state now live inside dedicated helper objects instead of being written straight into host-owned mutable
  properties.
- Because the root type remains `QtObject`, those helper collaborators are composed through explicit object properties
  rather than anonymous inline children.
  This keeps the controller valid under QML lint/compile rules while preserving the same public API.
