# `src/app/viewmodel/content/ContentsStructuredDocumentMutationPolicy.hpp`

## Responsibility
Declares the RAW mutation-policy surface consumed by `ContentsStructuredDocumentFlow.qml` for structured text editing.

## Current Behavior
- Exposes deletion-range helpers for empty text blocks.
- Exposes focus-safe insertion payload builders for generic structured blocks and resource-tag blocks.
- Exposes paragraph-boundary policy hooks so QML can ask whether a parsed block supports paragraph merge/split.
- Exposes paragraph merge and split payload builders that return the full next RAW source plus the focus request that
  should be restored after reparsing.

## Architecture Note
- This interface is intentionally QML-facing.
- Flow delegates describe boundary intent only; this policy owns the actual RAW rewrite contract.
