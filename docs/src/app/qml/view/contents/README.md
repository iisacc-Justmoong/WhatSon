# Contents View QML

This directory contains the only QML files allowed inside the contents-view namespace.

## Files

- `Gutter.qml`: view-only line number rail.
- `ImageEditor.qml`: image resource viewer surface.
- `TextEditor.qml`: read-only LVRS editor shell and line-metric provider.
- `Minimap.qml`: compact text minimap fed by `TextEditor.qml`.

## Contract

Do not reintroduce `DocumentEditor.qml`, placeholder resource editors, nested `editor/` directories, renderer wrappers, projection caches, or active note document-session adapters in this namespace.
