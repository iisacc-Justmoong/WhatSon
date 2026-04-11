# `src/app/agenda/ContentsAgendaBackend.hpp`

## Responsibility
Declares the agenda editor backend bridge exposed to QML as
`WhatSon.App.Internal/ContentsAgendaBackend`.

## Public QML Contract
- `parseAgendas(sourceText)`: parses canonical `<agenda>/<task>` source blocks into render-model entries.
- `rewriteTaskDoneAttribute(sourceText, taskOpenTagStart, taskOpenTagEnd, checked)`: rewrites one task open-tag
  `done` attribute in-place.
- `buildAgendaInsertionPayload(done, taskText)`: builds canonical agenda/task insertion source and cursor offset data.
- `detectTodoShortcutReplacement(previousPlainText, replacementStart, replacementEnd, insertedText)`: detects `[]` /
  `[x]` shorthand and returns canonical agenda replacement metadata.
- `detectAgendaTaskEnterReplacement(sourceText, sourceStart, sourceEnd, insertedText)`: applies agenda-internal
  `Enter` behavior (next task / exit agenda).
- `normalizeAgendaModifiedDate(sourceText)`: rewrites `date="yyyy-mm-dd"` placeholders to current `YYYY-MM-DD`.
- `todayIsoDate()`: exposes current local date token used by agenda insertion/normalization.

## Registration Constraint
- This QObject type is registered via `qmlRegisterType<ContentsAgendaBackend>()`.
- The class must remain non-`final` to keep compatibility with Qt's internal `QQmlElement<T>` wrapper type.
