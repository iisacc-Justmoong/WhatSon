# `src/app/editor/renderer/ContentsStructuredBlockRenderer.cpp`

## Responsibility
Builds agenda/callout render models for editor overlay layers from canonical `.wsnbody` source text.

## Key Behavior
- Reuses `ContentsAgendaBackend::parseAgendas(...)` and `ContentsCalloutBackend::parseCallouts(...)` so render-model
  parsing stays aligned with domain-owned source rules.
- Recomputes render data whenever `sourceText` changes.
- Emits one shared `renderedBlocksChanged()` signal only when agenda/callout render payloads actually change.
- Keeps the QML host contract renderer-owned:
  - QML views no longer call agenda/callout parse backends directly
  - desktop/mobile hosts bind `ContentsAgendaLayer` / `ContentsCalloutLayer` to this renderer's properties

## Regression Checks
- A source string containing `<agenda ...>` must produce agenda entries with task metadata through
  `renderedAgendas`.
- A source string containing `<callout>...</callout>` must produce callout entries through `renderedCallouts`.
- Unchanged source text must not emit redundant render-model updates.
