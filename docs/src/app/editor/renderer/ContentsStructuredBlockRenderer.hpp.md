# `src/app/editor/renderer/ContentsStructuredBlockRenderer.hpp`

## Responsibility
Declares the editor-side structured-block renderer that projects proprietary block tags into QML-facing render models.

## Public Contract
- `sourceText`: canonical RAW note-body source consumed by the renderer.
- `renderedAgendas`: `QVariantList` of parsed agenda/task card models.
- `renderedCallouts`: `QVariantList` of parsed callout row models.
- `renderedDocumentBlocks`: one ordered block stream for the structured editor host.
  Supported top-level body tags no longer need per-tag block list ownership at the renderer boundary; they are all
  projected into this same block sequence with common source-span geometry.
- `agendaParseVerification`: latest agenda parser verification report forwarded from `ContentsAgendaBackend`.
- `calloutParseVerification`: latest callout parser verification report forwarded from `ContentsCalloutBackend`.
- `structuredParseVerification`: merged renderer-level verification report that bundles agenda/callout/break
  verification plus the linter's synthetic XML/body well-formedness report for supported semantic tags.
  It exposes a top-level `wellFormed` bit, an `xml` child verification payload, and `canonicalizationSuggested` when
  the RAW source can be safely normalized by the file-layer linter.
- `correctedSourceText`: canonical RAW source projection suggested by the file-layer structured-tag linter.
- `correctionSuggested`: convenience bit telling QML whether the current source differs from that canonical projection.
- `backgroundRefreshEnabled`: lets QML move structured parse/canonicalization work off the UI thread when the editor is
  not actively focused.
- `renderPending`: tells QML that a worker-thread structured render is still in flight.
- `agendaCount` / `calloutCount` / `hasRenderedBlocks`: cheap QML visibility helpers.
- `hasNonResourceRenderedBlocks`: tells QML whether the parsed document contains block-flow structures that truly
  require the structured document host to replace the legacy inline editor.
  `resource` blocks are excluded so inline image/resource notes can stay on the ordinary editor surface unless some
  other block type such as `agenda`, `callout`, or `break` is present.
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
