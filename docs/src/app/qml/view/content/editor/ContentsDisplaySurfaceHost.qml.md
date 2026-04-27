# `src/app/qml/view/content/editor/ContentsDisplaySurfaceHost.qml`

## Responsibility

`ContentsDisplaySurfaceHost.qml` owns the center note-editor surface composition that used to live directly inside
`ContentsDisplayView.qml`.

It mounts the print/page viewport, the structured document viewport, `ContentsStructuredDocumentFlow.qml`,
read-side formatted preview text, the visual drag/drop affordance for resource imports, and the model-side input
command surface.

The host intentionally does not mount a note-loading overlay. Mount scheduling is settled by the display model's
`mountDecisionClean` flag instead of a visual mask, so rendered note content is never dimmed by a stale loading state.
The host also gates focus/input with `noteDocumentSurfaceInteractive`, not the raw `selectedNoteBodyLoading` flag, so
startup runtime loading cannot keep an already parse-mounted note editor disabled.

## Boundary

The host does not decide which surface mode is active and does not mutate note source on its own. It consumes state,
controllers, renderers, and callbacks provided by `ContentsDisplayView.qml` and delegates source changes back through
`applyDocumentSourceMutation(...)`.

The root display view receives aliases for the mounted viewport, structured flow, and command surface so existing
display controllers can keep using one active editor-surface contract while the visual tree remains outside the root
file.
