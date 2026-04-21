# `src/app/models/file/validator/WhatSonStructuredTagLinter.hpp`

## Role
`WhatSonStructuredTagLinter` owns note-body proprietary structured-tag linting, safe canonicalization, and
well-formedness verification for the supported semantic/body XML projection.

## Public API
- `normalizeStructuredSourceText(const QString&)`
  - Applies safe canonical rewrites for the proprietary body tags currently supported by the editor:
    - `</break>`
    - `<agenda date="...">...</agenda>`
    - `<task done="...">...</task>`
    - `<callout>...</callout>`
- `buildBreakVerification(const QString&)`
  - Reports canonical vs legacy divider-tag usage.
- `buildAgendaVerification(const QString&, int parsedAgendaCount, int parsedTaskCount, int invalidAgendaChildCount)`
  - Builds agenda/task verification payloads for parser callers using both raw-source lint signals and parser-confirmed counts.
- `buildCalloutVerification(const QString&, int parsedCalloutCount)`
  - Builds callout verification payloads for parser callers.
- `buildStructuredVerification(const QVariantMap&, const QVariantMap&, const QString&)`
  - Merges agenda/callout verification with break-tag lint and synthetic XML well-formedness verification into one
    renderer-friendly structured verification map.
  - The merged payload now includes an `xml` child verification for supported semantic/body tags such as `paragraph`,
    `title`, `subTitle`, `event*`, `resource`, `next`, hashtag tags, and inline style aliases.
  - XML parse issues also include parser location context and an approximate `sourceLineNumber` derived from preserved
    source line breaks in the synthetic body document.

## Layer Placement
- This type lives in `file/validator` because the rules describe canonical `.wsnbody` source validity, not editor-widget behavior.
- Parser backends and note persistence may both depend on it without moving body-tag business rules into QML.
