# `src/app/editor/renderer/ContentsStructuredBlockRenderer.hpp`

## Responsibility
Declares the editor-side structured-block renderer that projects proprietary block tags into QML-facing render models.

## Public Contract
- `sourceText`: canonical RAW note-body source consumed by the renderer.
- `renderedAgendas`: `QVariantList` of parsed agenda/task card models.
- `renderedCallouts`: `QVariantList` of parsed callout row models.
- `agendaCount` / `calloutCount` / `hasRenderedBlocks`: cheap QML visibility helpers.
- `requestRenderRefresh()`: explicit recompute hook for hosts that need a forced refresh after external state changes.

## Architectural Note
- This renderer lives in `editor/renderer` because tag detection and render-model projection belong to the editor
  presentation layer.
- Agenda/callout authoring and source-mutation APIs remain in `src/app/agenda` and `src/app/callout`; this type only
  exposes read-side render data.
