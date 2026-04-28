# `src/app/models/editor/tags/ContentsAgendaBackend.hpp`

## Responsibility
Declares the agenda editor backend bridge exposed to QML as
`WhatSon.App.Internal/ContentsAgendaBackend`.

## Public QML Contract
- `lastParseVerification`: latest parser verification report for agenda/task tags.
- `parseAgendas(sourceText)`: parses canonical `<agenda>/<task>` source blocks into render-model entries.
- `rewriteTaskDoneAttribute(sourceText, taskOpenTagStart, taskOpenTagEnd, checked)`: rewrites one task open-tag
  `done` attribute in-place.
- `detectTodoShortcutReplacement(previousPlainText, replacementStart, replacementEnd, insertedText)`: detects `[]` /
  `[x]` shorthand and returns canonical agenda replacement metadata.
- `detectAgendaTaskEnterReplacement(sourceText, sourceStart, sourceEnd, insertedText)`: applies agenda-internal
  `Enter` behavior (next task / exit agenda).
- `normalizeAgendaModifiedDate(sourceText)`: rewrites `date="yyyy-mm-dd"` placeholders to current `YYYY-MM-DD`.
- `todayIsoDate()`: exposes current local date token used by agenda normalization.

## Signals
- `lastParseVerificationChanged()`: emitted when the cached agenda parse verification report changes.
- `parseVerificationReported(verification)`: emitted on every agenda parse pass with counts/issues for
  `<agenda>` / `<task>` confirmation state.

## Registration Constraint
- This QObject type is registered through the LVRS manifest in `WhatSonQmlInternalTypeRegistrar`.
- The class must remain non-`final` to keep compatibility with Qt's internal `QQmlElement<T>` wrapper type.
