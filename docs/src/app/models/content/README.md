# `src/app/models/content`

## Responsibility
`src/app/models/content` owns non-editor content-surface helper objects that sit between app state and LVRS/QML hosts.

## Scope
- Mirrored source directory: `src/app/models/content`
- Child directories: 1
- Child files: 0

## Child Directories
- `mobile`

## Architectural Notes
- Editor-host display coordination now lives under `src/app/models/editor/display`.
- `mobile` owns mobile-only route planning, back-swipe, pop-repair, and hierarchy-selection preservation helpers that
  keep compact workspace navigation deterministic across note/detail/editor transitions.
- Structured editor host and policy types now live under `src/app/models/editor/structure`.

## Verification Notes
- Mobile content helpers are covered by the shared `whatson_cpp_regression` suite, including route-state restore and
  sidebar-binding resolution regression checks.
