# `src/app/models/editor/renderer/ContentsStructuredBlockRenderer.hpp`

## Responsibility
Declares the editor-side structured-block renderer that republishes parsed `.wsnbody` block projections into
QML-facing render models.

## Public Contract
- `sourceText`: canonical RAW note-body source consumed by the renderer.
- `renderedDocumentBlocks`: one ordered block stream for the structured editor host.
  Supported top-level body tags no longer need per-tag block list ownership at the renderer boundary; they are all
  projected into this same block sequence with common source-span geometry.
- `structuredParseVerification`: merged renderer-level verification report that bundles break verification plus the
  linter's synthetic XML/body well-formedness report for supported semantic tags.
  It exposes a top-level `wellFormed` bit, an `xml` child verification payload, and `canonicalizationSuggested` when
  the RAW source can be safely normalized by the file-layer linter.
- `correctedSourceText`: canonical RAW source projection suggested by the file-layer structured-tag linter.
- `correctionSuggested`: convenience bit telling QML whether the current source differs from that canonical projection.
- `backgroundRefreshEnabled`: lets QML move structured parse/canonicalization work off the UI thread when the editor is
  not actively focused.
- `renderPending`: tells QML that a worker-thread structured render is still in flight.
- `hasRenderedBlocks`: cheap QML visibility helper.
- `hasNonResourceRenderedBlocks`: tells QML whether the parsed document contains explicit blocks other than
  `resource`.
- `requestRenderRefresh()`: explicit recompute hook for hosts that need a forced refresh after external state changes.

## Signals
- `structuredParseVerificationChanged()`
- `correctedSourceTextChanged()` / `correctionSuggestedChanged()`
- `backgroundRefreshEnabledChanged()` / `renderPendingChanged()`
- `structuredParseVerificationReported(verification)`
- `structuredCorrectionSuggested(sourceText, correctedSourceText, verification)`

## Architectural Note
- This renderer lives in `editor/renderer` because render-model publication still belongs to the editor presentation
  layer, even though top-level block tokenization now starts in `editor/parser`.
- Agenda/task and callout authoring use ordinary paired-tag insertion in `src/app/models/editor/tags`; this type only
  exposes read-side render data derived from the parser result.
