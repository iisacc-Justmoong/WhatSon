# `src/app/agenda`

## Responsibility
Owns agenda-specific editor backend logic that QML editor surfaces consume through
`WhatSon.App.Internal/ContentsAgendaBackend`.

## Scope
- Source directory: `src/app/agenda`
- Child directories: none
- Child files:
  - `ContentsAgendaBackend.hpp`
  - `ContentsAgendaBackend.cpp`

## Contracts
- Provides QML-invokable agenda parser/model projection for `ContentsAgendaLayer.qml`.
- Provides source rewrite helpers for agenda task toggle (`done=true|false`) updates.
- Provides agenda shortcut mutation helpers used by `ContentsEditorTypingController.qml`:
  - direct agenda insertion (`Cmd+Opt+T`)
  - todo shorthand (`[] item` / `[x] item`) canonicalization into `<agenda>/<task>`
  - `Enter` handling inside `<task>` for next-task creation or agenda exit
- Provides modified-date placeholder normalization (`date="yyyy-mm-dd"` -> current `YYYY-MM-DD`) used by
  `ContentsEditorSession.qml`.

## Dependency Direction
- Depends only on Qt core types (`QObject`, `QRegularExpression`, `QVariantMap/List`, `QDate`).
- Does not depend on QML view types, note stores, or view-model layers.
