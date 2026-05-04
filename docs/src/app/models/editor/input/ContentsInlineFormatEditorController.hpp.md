# `src/app/models/editor/input/ContentsInlineFormatEditorController.hpp`

## Responsibility

Declares the C++ controller exposed to QML as `ContentsInlineFormatEditorController`.

## Contract

- Owns only the current inline editor wrapper contract.
- Delegates implementation details to `src/app/qml/view/contents/editor/ContentsInlineFormatEditorController.qml`.
- Exposes native-input, selection-cache, cursor-restore, and committed-text dispatch hooks for
  `ContentsInlineFormatEditor.qml`.
- Does not expose block-delegate row-coordinate or legacy structured-block controller APIs.
