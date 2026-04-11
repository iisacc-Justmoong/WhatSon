# `src/app/file/validator/WhatSonStructuredTagLinter.cpp`

## Responsibility
Implements linting and safe canonicalization for proprietary note-body structured tags.

## Safe Canonicalization Policy
- The linter only applies repairs that are safe and deterministic:
  - legacy divider aliases (`<break>`, `<break/>`, `<hr>`, `<hr/>`, `</hr>`) -> canonical `</break>`
  - self-closing `<callout .../>` -> `<callout></callout>`
  - self-closing `<task .../>` -> canonical `<task done="true|false"> </task>`
  - self-closing `<agenda .../>` -> canonical `<agenda date="YYYY-MM-DD"><task done="false"> </task></agenda>`
  - complete `<callout ...>...</callout>` blocks -> canonical `<callout>...</callout>`
  - complete `<agenda ...>...</agenda>` blocks -> canonical date/task attributes
- For taskless agendas, the linter inserts one empty canonical task only when the agenda body is empty or plain text.
- If a taskless agenda body already contains raw tag syntax, the linter preserves that body and reports the problem instead of making a destructive guess.

## Verification Payloads
- `buildAgendaVerification(...)` reports:
  - agenda/task open-close counts
  - parser-confirmed counts
  - invalid agenda child count
  - invalid/missing agenda dates
  - self-closing agenda/task counts
  - missing / non-canonical `task.done`
- `buildCalloutVerification(...)` reports:
  - open-close counts
  - parser-confirmed counts
  - self-closing callout count
  - non-canonical callout attribute usage
- `buildBreakVerification(...)` reports canonical vs legacy divider usage.
- `buildStructuredVerification(...)` merges agenda/callout/break verification and exposes `canonicalizationSuggested` when the source differs from the linter-normalized projection.

## Integration Points
- `WhatSonNoteBodyPersistence` uses the linter before `.wsnbody` serialization and when projecting stored body XML back into editor RAW source.
- `ContentsAgendaBackend` / `ContentsCalloutBackend` use the linter to build parser verification payloads.
- `ContentsStructuredBlockRenderer` uses the linter to merge renderer-level structured verification, including divider-tag lint.

## Regression Checklist
- Legacy divider aliases must round-trip into canonical `</break>` source.
- Self-closing agenda/task/callout tags must normalize into explicit block wrappers on save/load projection.
- Agenda verification must fail when date/done canonical requirements are violated even if open/close pairs still match.
- Callout verification must fail when custom attributes appear on `<callout>` start tags.
