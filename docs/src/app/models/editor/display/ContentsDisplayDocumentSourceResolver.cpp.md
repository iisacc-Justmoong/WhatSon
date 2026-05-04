# `ContentsDisplayDocumentSourceResolver.cpp`

- Implements the same-note source arbitration rules for note snapshot text, structured-flow source text, and the bound
  editor session text.
- The resolver is intentionally side-effect free: it computes presentation and mutation plans, while persistence and
  mount coordination remain elsewhere.
- Same-note evaluation is no longer based on `noteId` alone.
  When `selectedNoteDirectoryPath` is present, the resolver only treats the editor session as bound to the selection if
  `editorBoundNoteId == selectedNoteId` and `editorBoundNoteDirectoryPath == selectedNoteDirectoryPath`.
- The derived outputs that QML depends on most heavily now live behind signal-backed properties:
  `documentSourcePlan` and `documentPresentationSourceText`.
  This prevents the view from caching a one-shot `Q_INVOKABLE` result and then mounting against stale note-body state
  after the selection bridge resolves a fresh `.wsnbody`.
- A resolved selected-note snapshot now stays visible while the editor session is merely bound but still carries an
  empty `editorText`.
  This avoids the historical blank-document regression where minimap chrome appeared for the selected note
  but the document body collapsed to an empty paragraph until the session text caught up.
- A resolved selected-note snapshot also stays presentation-authoritative when the bound editor session still carries
  stale render-session text for the same note.
  The resolver now switches back to editor text only when there is no usable snapshot yet or when a pending body save
  marks the editor session as the deliberate local authority.
- A bound editor session no longer treats an empty placeholder session as presentation-ready by itself.
  Until the selected snapshot resolves, the resolver now keeps `resolvedSourceReady` false instead of synthesizing a
  blank render source from an unconfirmed same-note session bind.
- `documentSourcePlan` now also surfaces both `selectedNoteDirectoryPath` and `editorBoundNoteDirectoryPath`.
  QML mount logic can therefore diagnose "same id, different package" mismatches without inferring that distinction
  from stale text or timing.
- Explicit empty-body snapshots that still belong to the selected note remain `bodyAvailable` / `resolvedSourceReady`,
  so note-open flows can mount a blank document surface instead of surfacing a false "body unavailable" error.
