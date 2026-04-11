# `src/app/editor/renderer/ContentsStructuredBlockRenderer.cpp`

## Responsibility
Builds agenda/callout render models for editor overlay layers from canonical `.wsnbody` source text.

## Key Behavior
- Reuses `ContentsAgendaBackend::parseAgendas(...)` and `ContentsCalloutBackend::parseCallouts(...)` so render-model
  parsing stays aligned with domain-owned source rules.
- Recomputes render data whenever `sourceText` changes.
- Emits one shared `renderedBlocksChanged()` signal only when agenda/callout render payloads actually change.
- Listens to backend parser verification signals and forwards them as renderer-owned verification properties/signals:
  - `agendaParseVerification`
  - `calloutParseVerification`
  - `structuredParseVerification`
- Merges agenda/callout issue lists into one renderer-level verification payload so QML/debug tooling can inspect
  whether the current structured RAW source was actually confirmed by the parsers.
- Uses `WhatSonStructuredTagLinter::buildStructuredVerification(...)` for the merged payload, so renderer-level
  verification also includes divider-tag lint (`break`) and a `canonicalizationSuggested` bit.
- Seeds empty agenda/callout verification maps as `wellFormed=true` defaults before the first parse result arrives,
  preventing the merged renderer-level status from transiently reporting a false failure while only one backend has
  emitted.
- Keeps the QML host contract renderer-owned:
  - QML views no longer call agenda/callout parse backends directly
  - desktop/mobile hosts bind `ContentsAgendaLayer` / `ContentsCalloutLayer` to this renderer's properties

## Regression Checks
- A source string containing `<agenda ...>` must produce agenda entries with task metadata through
  `renderedAgendas`.
- A source string containing `<callout>...</callout>` must produce callout entries through `renderedCallouts`.
- Malformed agenda/task/callout source must surface through parser-verification signals even when renderable block
  payloads did not otherwise change.
- Unchanged source text must not emit redundant render-model updates.
