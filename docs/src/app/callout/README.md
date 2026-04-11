# `src/app/callout`

## Responsibility
Owns callout-specific editor backend logic consumed by QML editor surfaces through
`WhatSon.App.Internal/ContentsCalloutBackend`.

## Scope
- Source directory: `src/app/callout`
- Child directories: none
- Child files:
  - `ContentsCalloutBackend.hpp`
  - `ContentsCalloutBackend.cpp`

## Contracts
- Provides QML-invokable callout parser/model projection for `ContentsCalloutLayer.qml`.
- Provides canonical callout insertion payloads used by `ContentsEditorTypingController.qml` (`Cmd+Opt+C`).
- Provides callout exit rewrite detection for `Enter` handling so an empty trailing callout line can leave the block.

## Dependency Direction
- Depends only on Qt core types (`QObject`, `QRegularExpression`, `QVariantMap/List`).
- Does not depend on QML view types, note stores, or view-model layers.
