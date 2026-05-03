# `src/app/qml/view/contents/editor/ContentsDisplaySurfaceHost.qml`

## Responsibility

`ContentsDisplaySurfaceHost.qml` owns the center note-editor surface composition that used to live directly inside
`ContentsDisplayView.qml`.

It mounts the print/page viewport, the structured document viewport, `ContentsStructuredDocumentFlow.qml`,
read-side formatted preview text, the visual drag/drop affordance for resource imports, and the model-side input
command surface.

The host intentionally does not mount a note-loading overlay. Mount scheduling is settled by the display model's
`mountDecisionClean` flag instead of a visual mask, so rendered note content is never dimmed by a stale loading state.
The host accepts focus/input as soon as a note is selected and no mount exception is visible. Parsed RAW state still
controls block content and tag-command surfaces, but it must not be a prerequisite for accepting the first
click/touch.

The host forwards left taps from both the structured viewport and the full-editor activation surface to the structured
flow's trailing-margin hit test. When the tap lands below the rendered document body, the host requests the same focus
behavior as clicking the note body's terminal position.
That accessibility contract explicitly includes freshly created empty notes whose body is represented by the renderer's
single empty `text-group`, but it does not treat side whitespace beside existing lines as a body-end click.
The full-editor activation path may pass points that fall outside the current structured viewport; the host clamps
those coordinates back onto the viewport before applying the same trailing-margin policy.
Those two pointer paths coalesce through a one-turn queue guard so a single click cannot emit duplicate terminal body
click requests.
The maintained C++/QML regression test now exercises both the direct host helper and an actual `QQuickWindow` left
click in the bottom inset, and it asserts that the live inline editor receives focus with the cursor at the source
body end.

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
