# `src/app/qml/view/content/editor/ContentsResourceImportConflictController.qml`

## Role
`ContentsResourceImportConflictController.qml` owns duplicate-resource decision flow for editor imports.

## Responsibilities
- Decides whether a dropped URL list or clipboard image can be imported on the current host.
- Stores pending duplicate-import state locally instead of mutating transient alert state on the host view.
- Executes `Overwrite`, `Keep Both`, or `Cancel Import` follow-up work and coordinates guard release plus tag insertion
  through collaborator controllers.
