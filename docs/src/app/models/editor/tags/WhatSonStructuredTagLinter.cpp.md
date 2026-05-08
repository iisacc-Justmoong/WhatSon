# `src/app/models/editor/tags/WhatSonStructuredTagLinter.cpp`

## Responsibility
Implements linting and safe canonicalization for proprietary note-body structured tags, then validates that supported
semantic/body tags still form a well-formed XML projection.

## Safe Canonicalization Policy
- The linter only applies repairs that are safe and deterministic:
  - legacy divider aliases (`<break>`, `<break/>`, `<hr>`, `<hr/>`, `</hr>`) -> canonical `</break>`
- XML verification uses the linter-normalized source projection so safe break repairs happen before the
  synthetic `<contents><body>...</body></contents>` document is parsed.
- The synthetic XML projection recognizes the same supported body semantics as note-body persistence: resource tags,
  `paragraph`/`p`/heading-style block tags, `next`, `event` wrappers, `eventTitle`/`eventDescription`, hashtag tags,
  and inline style aliases.
- The synthetic XML projection now preserves source line breaks so parser error line numbers stay useful for malformed
  multi-line `.wsnbody` input instead of collapsing back to one synthetic line.

## Verification Payloads
- `buildBreakVerification(...)` reports canonical vs legacy divider usage.
- `buildStructuredVerification(...)` merges break verification with a synthetic XML well-formedness report exposed
  under `xml`.
- The `xml` verification fails when supported semantic/body tags cannot be parsed into one well-formed body document
  after safe normalization, and reports `body_xml_not_well_formed` with parser line/column context plus an approximate
  `sourceLineNumber`.
- `buildStructuredVerification(...)` still exposes `canonicalizationSuggested` when the source differs from the
  linter-normalized projection.

## Integration Points
- `WhatSonNoteBodyPersistence` owns `.wsnbody` body-format serialization directly; the linter remains the editor-domain
  correction and verification surface for already loaded RAW source.
- `ContentsStructuredBlockRenderer` republishes parser-level structured verification, including divider lint plus
  generic semantic/body XML well-formedness.
- `ContentsStructuredTagValidator` consumes renderer correction suggestions and reports advisory correction state
  without making QML the source of truth for note writes.

## Regression Checklist
- Legacy divider aliases must round-trip into canonical `</break>` source.
- Agenda/task and callout attributes are not linted by tag-specific verifiers; `<agenda><task>...</task></agenda>` and
  `<callout ...>...</callout>` are validated only as ordinary XML-like RAW source.
- Malformed `paragraph`/`title`/`subTitle`/`event*`/`resource` source markup must now fail the `xml` verification
  instead of silently passing structured lint.
