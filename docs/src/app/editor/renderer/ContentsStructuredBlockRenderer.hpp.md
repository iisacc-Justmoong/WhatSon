# `src/app/editor/renderer/ContentsStructuredBlockRenderer.hpp`

## Responsibility
Declares the editor-side structured-block renderer that projects proprietary block tags into QML-facing render models.

## Public Contract
- `sourceText`: canonical RAW note-body source consumed by the renderer.
- `renderedAgendas`: `QVariantList` of parsed agenda/task card models.
- `renderedCallouts`: `QVariantList` of parsed callout row models.
- `agendaParseVerification`: latest agenda parser verification report forwarded from `ContentsAgendaBackend`.
- `calloutParseVerification`: latest callout parser verification report forwarded from `ContentsCalloutBackend`.
- `structuredParseVerification`: merged renderer-level verification report that bundles agenda/callout/break
  verification, a top-level `wellFormed` bit, and `canonicalizationSuggested` when the RAW source can be safely
  normalized by the file-layer linter.
- `correctedSourceText`: canonical RAW source projection suggested by the file-layer structured-tag linter.
- `correctionSuggested`: convenience bit telling QML whether the current source differs from that canonical projection.
- `backgroundRefreshEnabled`: lets QML move structured parse/canonicalization work off the UI thread when the editor is
  not actively focused.
- `renderPending`: tells QML that a worker-thread structured render is still in flight.
- `agendaCount` / `calloutCount` / `hasRenderedBlocks`: cheap QML visibility helpers.
- `requestRenderRefresh()`: explicit recompute hook for hosts that need a forced refresh after external state changes.

## Signals
- `agendaParseVerificationChanged()` / `calloutParseVerificationChanged()` / `structuredParseVerificationChanged()`
- `correctedSourceTextChanged()` / `correctionSuggestedChanged()`
- `backgroundRefreshEnabledChanged()` / `renderPendingChanged()`
- `agendaParseVerificationReported(verification)` / `calloutParseVerificationReported(verification)` /
  `structuredParseVerificationReported(verification)`
- `structuredCorrectionSuggested(sourceText, correctedSourceText, verification)`

## Architectural Note
- This renderer lives in `editor/renderer` because tag detection and render-model projection belong to the editor
  presentation layer.
- Agenda/callout authoring and source-mutation APIs remain in `src/app/agenda` and `src/app/callout`; this type only
  exposes read-side render data.
