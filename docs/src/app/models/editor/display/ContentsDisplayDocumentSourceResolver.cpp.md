# `ContentsDisplayDocumentSourceResolver.cpp`

- Implements the same-note source arbitration rules for note snapshot text, structured-flow source text, and the bound
  editor session text.
- The resolver is intentionally side-effect free: it computes presentation and mutation plans, while persistence and
  mount coordination remain elsewhere.
- The derived outputs that QML depends on most heavily now live behind signal-backed properties:
  `documentSourcePlan` and `documentPresentationSourceText`.
  This prevents the view from caching a one-shot `Q_INVOKABLE` result and then mounting against stale note-body state
  after the selection bridge resolves a fresh `.wsnbody`.
- Explicit empty-body snapshots that still belong to the selected note remain `bodyAvailable` / `resolvedSourceReady`,
  so note-open flows can mount a blank document surface instead of surfacing a false "body unavailable" error.
