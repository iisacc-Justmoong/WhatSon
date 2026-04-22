# `ContentsDisplayDocumentSourceResolver.hpp`

- Declares the QML-facing editor source resolver that normalizes selected-note snapshot state against the live editor
  session.
- Exposes both raw editor/snapshot inputs and derived, signal-backed read models for QML:
  `documentSourcePlan` and `documentPresentationSourceText`.
- Lives under `models/editor/display` so note-open source arbitration is treated as editor-domain logic, not as a
  generic content-panel concern.
