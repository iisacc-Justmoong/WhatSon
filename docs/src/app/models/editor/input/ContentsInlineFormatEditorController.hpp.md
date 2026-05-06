# `src/app/models/editor/input/ContentsInlineFormatEditorController.hpp`

## Responsibility

Declares the C++ controller exposed to QML as `ContentsInlineFormatEditorController`.

## Contract

- Owns only the current inline editor wrapper contract.
- Implements input policy and synchronization state directly in C++; there is no QML helper for this controller.
- Exposes native-input, selection-cache, cursor-restore, and committed-text dispatch hooks for
  `ContentsInlineFormatEditor.qml`.
- Does not expose or install a key event-filter override; ordinary text-edit keys remain native input.
- Does not expose block-delegate row-coordinate or legacy structured-block controller APIs.
