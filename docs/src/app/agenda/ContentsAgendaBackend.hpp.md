# `src/app/agenda/ContentsAgendaBackend.hpp`

## Responsibility
Declares the agenda editor backend bridge exposed to QML as
`WhatSon.App.Internal/ContentsAgendaBackend`.

## Public QML Contract
- `lastParseVerification`: latest parser verification report for agenda/task tags.
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

## Signals
- `lastParseVerificationChanged()`: emitted when the cached agenda parse verification report changes.
- `parseVerificationReported(verification)`: emitted on every agenda parse pass with counts/issues for
  `<agenda>` / `<task>` confirmation state.

## Registration Constraint
- This QObject type is registered via `qmlRegisterType<ContentsAgendaBackend>()`.
- The class must remain non-`final` to keep compatibility with Qt's internal `QQmlElement<T>` wrapper type.
