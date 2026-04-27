# `src/app/qml/view/content/editor/ContentsDisplaySurfaceHost.qml`

## Responsibility

`ContentsDisplaySurfaceHost.qml` owns the center note-editor surface composition that used to live directly inside
`ContentsDisplayView.qml`.

It mounts the print/page viewport, the structured document viewport, `ContentsStructuredDocumentFlow.qml`,
read-side formatted preview text, the visual drag/drop affordance for resource imports, and the model-side input
command surface.

The host intentionally does not mount a note-loading overlay. Mount scheduling is settled by the display model's
`mountDecisionClean` flag instead of a visual mask, so rendered note content is never dimmed by a stale loading state.
The host now gates focus/input with `noteDocumentParseMounted`; once RAW source has been parsed into the structured
document, late surface-ready bookkeeping cannot keep the visible editor disabled.

The host forwards left taps in the bottom document padding to the structured flow's document-end hit test from both
the surface-level pointer path and the structured viewport path. When the tap is below the last block, the host
requests the same focus behavior as clicking the note body's final editable position.
Those two pointer paths coalesce through a one-turn queue guard so a single click cannot emit duplicate document-end
edit requests.

The model-side input command surface must bind through `surfaceHost.contentsView` and
`surfaceHost.resourceImportController`. Those qualified bindings avoid a self-referential QML binding loop and keep
window-level formatting shortcuts connected to the live editor host.

## Boundary

The host does not decide which surface mode is active and does not mutate note source on its own. It consumes state,
controllers, renderers, and callbacks provided by `ContentsDisplayView.qml` and delegates source changes back through
`applyDocumentSourceMutation(...)`.

The root display view receives aliases for the mounted viewport, structured flow, and command surface so existing
display controllers can keep using one active editor-surface contract while the visual tree remains outside the root
file.
