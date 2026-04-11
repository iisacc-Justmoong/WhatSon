# `src/app/agenda/ContentsAgendaBackend.cpp`

## Responsibility
Implements agenda source parsing and mutation helpers shared by desktop/mobile editor surfaces.

## Key Behaviors
- Parses `<agenda ...>...</agenda>` blocks and nested `<task ...>...</task>` rows into QML-friendly `QVariantList`
  entries (`date`, `sourceStart`, `focusSourceOffset`, `tasks`, `done`, `text`, source open-tag offsets).
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
- Decodes safe entities (`&lt;`, `&gt;`, `&amp;`, `&quot;`, `&#39;`, `&nbsp;`) for agenda render text projection.
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
- Task toggle rewrite must only mutate the targeted open-tag `done` value.
- Typing `[] task` / `[x] task` must produce canonical agenda/task source replacement payloads.
- Empty agenda insertion payloads must include a one-space task anchor and place cursor at task-body start.
- Pressing `Enter` in an empty task must exit agenda editing according to source context.
- Pressing `Enter` in an empty task while all sibling tasks are also empty must remove the entire agenda block.
- `date="yyyy-mm-dd"` must normalize to `YYYY-MM-DD` during modification staging.
- macOS libc++ builds must compile this file without `std::clamp/std::max` deduced-type conflicts (`int` vs
  `qsizetype`).
