# `src/app/models/editor/input/ContentsAgendaBlockController.qml`

## Responsibility

Owns non-visual aggregate agenda block queries for `ContentsAgendaBlock.qml`.

The agenda view draws task rows. This controller resolves cross-row focus queries, selection cleanup, visible text, and
logical line layout aggregation.

## Contract

- Task-row typing state lives in `ContentsAgendaTaskRowController.qml`.
- The aggregate controller exposes block-level queries consumed by `ContentsDocumentBlock.qml` and
  `ContentsStructuredDocumentFlow.qml`.
- It does not mutate RAW source directly.

## Boundary

This controller should stay an aggregate block helper. Row-level text, toggle, and cursor state belongs to the task row
controller.
