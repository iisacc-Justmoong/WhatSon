# `src/app/agenda/ContentsAgendaBackend.cpp`

## Responsibility
Implements agenda source parsing and mutation helpers shared by desktop/mobile editor surfaces.

## Key Behaviors
- Parses `<agenda ...>` openings into QML-friendly `QVariantList` entries even before a matching `</agenda>` arrives,
  so page/print/web/presentation renderers can show a provisional agenda card immediately while the user is still
  authoring RAW tags.
- Parses nested `<task ...>` openings inside that agenda span with the same provisional rule: a task row is rendered as
  soon as a `<task>` start tag is visible, even if `</task>` has not been authored yet.
- Emits one synthetic placeholder task row when an agenda opening exists without any parsed task opening yet, keeping an
  empty agenda card visible instead of dropping the block entirely.
- Returns QML-friendly render entries (`date`, `sourceStart`, `focusSourceOffset`, `tasks`, `done`, `text`, source
  open-tag offsets, `tagVerified`, `hasSourceTag`).
- Publishes parser verification reports after every parse pass:
  - `agendaOpenCount`, `agendaCloseCount`, `agendaParsedCount`
  - `taskOpenCount`, `taskCloseCount`, `taskParsedCount`
  - `invalidAgendaChildCount`
  - invalid/missing `agenda.date`
  - self-closing `agenda` / `task`
  - missing / non-canonical `task.done`
  - `wellFormed`
  - `issues`
- Treats non-task residual content inside a parsed agenda body as an invalid-child verification failure, matching the
  architecture rule that agenda may contain only `task` children.
- Separates render visibility from structural verification:
  - provisional/open-only agenda cards are still returned to QML
  - `agendaParsedCount` / `taskParsedCount` remain reserved for fully closed pairs confirmed by the parser
  - incomplete blocks therefore stay visible while `wellFormed=false` still reports the mismatch to validator/runtime
- Delegates verification-map construction to `file/validator/WhatSonStructuredTagLinter`, keeping agenda canonical
  syntax rules in the file/domain validation layer.
- Rewrites one task open-tag `done` attribute while preserving the rest of the source.
- Builds canonical agenda insertion payloads:
  - `<agenda date="YYYY-MM-DD"><task done="true|false">...</task></agenda>`
  - when inserted task body is empty, writes a single-space edit anchor (`<task ...> </task>`) so the editor cursor
    can enter task scope through logical/source offset mapping.
  - for that empty-anchor case, cursor placement targets task-body start (before the anchor), keeping first typing
    input natural and preserving empty-task Enter-exit behavior.
- Detects todo shorthand edits from plain-text deltas and returns source replacement metadata.
- Detects `Enter` behavior inside `<task>`:
  - non-empty task -> append `</task><task done="false">`
  - empty task -> exit agenda context (or collapse whole agenda if it is the only task)
  - when exiting from an empty task, if every task in the agenda is empty, removes the whole `<agenda>...</agenda>`
    block to keep source body clean
- Normalizes modification placeholder date tokens (`yyyy-mm-dd`) to current local ISO date.
- Decodes safe entities (`&lt;`, `&gt;`, `&amp;`, `&quot;`, `&#39;`, `&nbsp;`) for agenda render text projection and
  strips nested inline tags from checkbox labels so provisional task rows still display readable plain text.
- Emits one agenda-level `focusSourceOffset` that points at the first task body, so renderer-hosted cards can return
  the editor cursor to the correct RAW insertion slot when the user taps the card.
- Uses explicit `qsizetype` -> `int` bounding helpers before `std::clamp/std::max` calls, preventing libc++
  template-deduction failures on macOS toolchains where `QString::size()` and regex capture offsets are `qsizetype`.
- Uses a custom raw-string delimiter for the `done`-attribute regex, preventing accidental raw-literal early
  termination by `)"` substrings inside the pattern body.

## Architectural Notes
- Agenda parsing/mutation regex and attribute handling are intentionally localized here so QML controllers only manage
  event flow and cursor orchestration.
- The module does not perform persistence directly; it only returns transformed source text or replacement metadata.

## Regression Checklist
- `<agenda date="2026-04-11"><task done="false">A</task></agenda>` must parse as one agenda + one unchecked task.
- Empty agendas that still contain one empty `<task>` body anchor must still parse into one visible agenda card model.
- `<agenda date="2026-04-11">` must still parse into one visible agenda card model with one provisional task row.
- `<agenda date="2026-04-11"><task done="false">A` must still render one agenda card with one task row while
  verification reports the missing close tags.
- Parser verification must report `wellFormed=false` when agenda/task open-close counts diverge or when parsed agenda
  bodies contain non-task child content.
- Task toggle rewrite must only mutate the targeted open-tag `done` value.
- Typing `[] task` / `[x] task` must produce canonical agenda/task source replacement payloads.
- Empty agenda insertion payloads must include a one-space task anchor and place cursor at task-body start.
- Pressing `Enter` in an empty task must exit agenda editing according to source context.
- Pressing `Enter` in an empty task while all sibling tasks are also empty must remove the entire agenda block.
- `date="yyyy-mm-dd"` must normalize to `YYYY-MM-DD` during modification staging.
- macOS libc++ builds must compile this file without `std::clamp/std::max` deduced-type conflicts (`int` vs
  `qsizetype`).
