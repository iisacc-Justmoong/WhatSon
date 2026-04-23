# `src/app/models/content`

## Responsibility
`src/app/models/content` owns content-surface helper objects that sit between raw note/body state and the LVRS/QML
hosts that render or navigate that state.

## Scope
- Mirrored source directory: `src/app/models/content`
- Child directories: 3
- Child files: 0

## Child Directories
- `display`
- `mobile`
- `structured`

## Architectural Notes
- `display` owns editor-host coordination that reacts to selection, context-menu, mount, and presentation refresh
  events without turning QML into a stateful document controller.
- Line-offset lookup, viewport correction math, and minimap proportional geometry now live under
  `src/app/models/editor/display` so the note editor does not keep those calculations inline in QML.
- `mobile` owns mobile-only route planning, back-swipe, pop-repair, and hierarchy-selection preservation helpers that
  keep compact workspace navigation deterministic across note/detail/editor transitions.
- `structured` owns the canonical structured document host, collection policy, focus policy, and mutation policy used
  by `ContentsStructuredDocumentFlow.qml`.
- The structured collection policy must normalize both C++ `QVariantList` payloads and QML `QJSValue` arrays.
  If that normalization drops JS array entries, the structured host will still mount gutter/minimap chrome while the
  document body collapses to `blockCount=0`.

## Verification Notes
- Mobile content helpers are covered by the shared `whatson_cpp_regression` suite, including route-state restore and
  sidebar-binding resolution regression checks.
