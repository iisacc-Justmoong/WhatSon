# `src/app/qml/view/panels/ContentViewLayout.qml`

`ContentViewLayout.qml` composes the central workspace content surface.

## Current Contract

- It mounts only `Gutter.qml`, `ImageEditor.qml`, `TextEditor.qml`, and `Minimap.qml` from `src/app/qml/view/contents`.
- Image resources selected through `noteListModel.currentResourceEntry` route to `ImageEditor.qml`.
- Non-image or note routes show a blank read-only `LV.TextEditor` shell for layout continuity.
- Calendar overlays temporarily replace the editor surface and route note-open requests back through the library hierarchy controller.

## Deleted Document Model Boundary

The active note document model was removed. This layout no longer receives or forwards editor document session objects, editor paste bridges, native key filters, RAW push/pull hooks, inline formatting shortcuts, or selection context menus.

QML in this file must not parse `.wsnbody`, mount active note body files, mutate source tags, or reintroduce document-session compatibility wrappers.
