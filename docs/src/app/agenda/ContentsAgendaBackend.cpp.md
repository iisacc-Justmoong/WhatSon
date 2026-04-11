# `src/app/agenda/ContentsAgendaBackend.cpp`

## Responsibility
Owns agenda-domain parsing and source rewrites for `<agenda>` / `<task>` blocks.

## Current Contract
- Agenda parse entries now expose full block/source geometry in addition to display text:
  - agenda: `sourceStart`, `sourceEnd`, `openTagStart`, `openTagEnd`, `contentStart`, `contentEnd`,
    `closeTagStart`, `closeTagEnd`, `hasCloseTag`
  - task: `openTagStart`, `openTagEnd`, `contentStart`, `contentEnd`, `closeTagStart`, `closeTagEnd`,
    `hasCloseTag`
- Empty agendas still emit one placeholder task entry so the editor can render a visible empty card as soon as the
  tag exists in RAW.
- `detectAgendaTaskEnterReplacement(...)` remains the canonical exit/continue rule for agenda editing:
  - non-empty task + Enter inserts the next `<task>`
  - empty trailing task + Enter exits the agenda
  - all-empty agenda + Enter removes the whole agenda block

## Why It Changed
The editor no longer treats agenda cards as detached overlay paint only. The document-flow QML editor now needs exact
source ranges so it can rewrite task text, toggle `done`, and reuse the backend's enter/exit semantics while keeping
RAW tags and visible cards synchronized.
