# `src/app/agenda/ContentsAgendaBackend.cpp`

## Responsibility
Implements agenda source parsing and mutation helpers shared by desktop/mobile editor surfaces.

## Key Behaviors
- Parses `<agenda ...>...</agenda>` blocks and nested `<task ...>...</task>` rows into QML-friendly `QVariantList`
  entries (`date`, `tasks`, `done`, `text`, source open-tag offsets).
- Rewrites one task open-tag `done` attribute while preserving the rest of the source.
- Builds canonical agenda insertion payloads:
  - `<agenda date="YYYY-MM-DD"><task done="true|false">...</task></agenda>`
  - cursor placement offset inside the task body.
- Detects todo shorthand edits from plain-text deltas and returns source replacement metadata.
- Detects `Enter` behavior inside `<task>`:
  - non-empty task -> append `</task><task done="false">`
  - empty task -> exit agenda context (or collapse whole agenda if it is the only task)
  - when exiting from an empty task, if every task in the agenda is empty, removes the whole `<agenda>...</agenda>`
    block to keep source body clean
- Normalizes modification placeholder date tokens (`yyyy-mm-dd`) to current local ISO date.
- Decodes safe entities (`&lt;`, `&gt;`, `&amp;`, `&quot;`, `&#39;`, `&nbsp;`) for agenda render text projection.

## Architectural Notes
- Agenda parsing/mutation regex and attribute handling are intentionally localized here so QML controllers only manage
  event flow and cursor orchestration.
- The module does not perform persistence directly; it only returns transformed source text or replacement metadata.

## Regression Checklist
- `<agenda date="2026-04-11"><task done="false">A</task></agenda>` must parse as one agenda + one unchecked task.
- Task toggle rewrite must only mutate the targeted open-tag `done` value.
- Typing `[] task` / `[x] task` must produce canonical agenda/task source replacement payloads.
- Pressing `Enter` in an empty task must exit agenda editing according to source context.
- Pressing `Enter` in an empty task while all sibling tasks are also empty must remove the entire agenda block.
- `date="yyyy-mm-dd"` must normalize to `YYYY-MM-DD` during modification staging.
