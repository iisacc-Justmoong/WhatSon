# `ContentsDisplayDocumentSourceResolver.hpp`

- Declares the QML-facing editor source resolver that normalizes selected-note snapshot state against the live editor
  session.
- Exposes both raw editor/snapshot inputs and derived, signal-backed read models for QML:
  `documentSourcePlan` and `documentPresentationSourceText`.
- The raw contract now also includes `selectedNoteDirectoryPath` and `editorBoundNoteDirectoryPath`, so source
  arbitration can distinguish same-id duplicate `.wsnote` packages.
- The resolver contract preserves a resolved selected-note snapshot as the presentation source until the bound editor
  session needs to represent a pending-save local edit or there is no usable snapshot yet.
- `documentSourcePlan.resolvedSourceReady` now tracks only presentation-ready sources.
  A same-note bind with an empty editor session is not considered ready unless a pending local save explicitly makes the
  editor the source of truth.
- Session/source matching therefore uses package identity when available: same `noteId` with a different resolved
  directory is treated as unbound selection state.
- Lives under `models/editor/display` so note-open source arbitration is treated as editor-domain logic, not as a
  generic content-panel concern.
