# `ContentsDisplayDocumentSourceResolver.hpp`

- Declares the QML-facing editor source resolver that normalizes selected-note snapshot state against the live editor
  session.
- Lives under `models/editor/display` so note-open source arbitration is treated as editor-domain logic, not as a
  generic content-panel concern.
