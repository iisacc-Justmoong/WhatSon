# `src/app/models/file/validator/WhatSonStructuredTagLinter.cpp`

## Responsibility
Implements linting and safe canonicalization for proprietary note-body structured tags, then validates that supported
semantic/body tags still form a well-formed XML projection.

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
- XML verification uses the linter-normalized source projection so safe agenda/callout/break repairs happen before the
  synthetic `<contents><body>...</body></contents>` document is parsed.
- The synthetic XML projection recognizes the same supported body semantics as note-body persistence: resource tags,
  `paragraph`/`p`/heading-style block tags, `next`, `event` wrappers, `eventTitle`/`eventDescription`, hashtag tags,
  and inline style aliases.
- The synthetic XML projection now preserves source line breaks so parser error line numbers stay useful for malformed
  multi-line `.wsnbody` input instead of collapsing back to one synthetic line.

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
- `buildStructuredVerification(...)` merges agenda/callout/break verification with a synthetic XML well-formedness
  report exposed under `xml`.
- The `xml` verification fails when supported semantic/body tags cannot be parsed into one well-formed body document
  after safe normalization, and reports `body_xml_not_well_formed` with parser line/column context plus an approximate
  `sourceLineNumber`.
- `buildStructuredVerification(...)` still exposes `canonicalizationSuggested` when the source differs from the
  linter-normalized projection.

## Integration Points
- `WhatSonNoteBodyPersistence` uses the linter before `.wsnbody` serialization and when projecting stored body XML back into editor RAW source.
- `ContentsAgendaBackend` / `ContentsCalloutBackend` use the linter to build parser verification payloads.
- `ContentsStructuredBlockRenderer` uses the linter to merge renderer-level structured verification, including divider
  lint plus generic semantic/body XML well-formedness.
- `ContentsStructuredTagValidator` consumes the renderer's corrected-source suggestion and persists the canonical
  output directly back into the note package.

## Regression Checklist
- Legacy divider aliases must round-trip into canonical `</break>` source.
- Self-closing agenda/task/callout tags must normalize into explicit block wrappers on save/load projection.
- Agenda verification must fail when date/done canonical requirements are violated even if open/close pairs still match.
- Callout verification must fail when custom attributes appear on `<callout>` start tags.
- Malformed `paragraph`/`title`/`subTitle`/`event*`/`resource` source markup must now fail the `xml` verification
  instead of silently passing structured lint.
