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
- `display` owns editor-host coordination that reacts to selection, viewport, minimap, gutter, and presentation
  refresh events without turning QML into a stateful document controller.
- `mobile` owns mobile-only route planning, back-swipe, pop-repair, and hierarchy-selection preservation helpers that
  keep compact workspace navigation deterministic across note/detail/editor transitions.
- `structured` owns the canonical structured document host, collection policy, focus policy, and mutation policy used
  by `ContentsStructuredDocumentFlow.qml`.

## Verification Notes
- Mobile content helpers are covered by the shared `whatson_cpp_regression` suite, including route-state restore and
  sidebar-binding resolution regression checks.
