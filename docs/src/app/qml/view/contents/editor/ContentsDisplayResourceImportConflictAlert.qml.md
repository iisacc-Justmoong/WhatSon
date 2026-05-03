# `src/app/qml/view/contents/editor/ContentsDisplayResourceImportConflictAlert.qml`

## Responsibility

Renders the duplicate-resource import conflict alert for the note editor surface.

## Boundary

- Owns only the LVRS alert shell and its visible command buttons.
- Delegates conflict policy execution and cancellation to `ContentsResourceImportController.qml`.
- Does not parse drops, create resource packages, mutate note source, or own import conflict state.
