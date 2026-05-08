# `src/app/models/editor/input/ContentsInlineFormatEditorController.hpp`

## Responsibility

Declares the C++ controller exposed to QML as `ContentsInlineFormatEditorController`.

## Contract

- Owns only the current inline editor wrapper contract.
- Implements input policy and synchronization state directly in C++; there is no QML helper for this controller.
- Exposes native-input, selection-cache, cursor-restore, and committed-text dispatch hooks for
  `ContentsInlineFormatEditor.qml`.
- Forwards platform shortcut sequence generation and native-to-standard shortcut event normalization from
  `ContentsEditorInputPolicyAdapter` so QML does not hard-code Command/Ctrl policy.
- Forwards text-aware tag-management shortcut requests so the QML view passes native `KeyEvent.text` through to C++
  without interpreting macOS Option-produced symbols itself.
- Exposes the native editor-item event-filter override only for explicit tag-management shortcuts; ordinary text-edit
  keys remain native input.
- Does not expose block-delegate row-coordinate or legacy structured-block controller APIs.
