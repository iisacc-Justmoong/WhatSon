# `src/app/qml`

The QML tree owns view composition only. Backend logic, persistence, parsing, source mutation, and document-model state stay out of QML.

## Current Contents Surface

`view/panels/ContentViewLayout.qml` composes the workspace content area with:

- `view/contents/Gutter.qml`
- `view/contents/ImageEditor.qml`
- `view/contents/TextEditor.qml`
- `view/contents/Minimap.qml`

The active editor document model was deleted. QML no longer receives a document session, editor paste bridge, or native key filter, and it must not reconstruct those responsibilities as compatibility wrappers.
