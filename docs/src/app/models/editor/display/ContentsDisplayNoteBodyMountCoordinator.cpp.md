# `ContentsDisplayNoteBodyMountCoordinator.cpp`

- Coordinates note-open mount decisions between selected-note snapshots, the bound editor session, and the active
  document surface.
- Mount readiness now follows the same presentation-ready source contract as
  `ContentsDisplayDocumentSourceResolver`.
  The coordinator no longer treats a merely bound, empty editor session as mountable for the selected note unless a
  pending local save explicitly makes that empty editor state authoritative.
- Explicit empty-body snapshots that are resolved for the selected note are treated as valid mountable sources, so the
  editor can open a blank note without falling back to the `"Note body unavailable"` exception surface.
- A selected note is no longer considered fully mounted merely because a presentation-ready source exists.
  The coordinator now keeps `noteMounted` false until the bound editor session is synchronized to that selected-note
  source, even when the note id already matches.
  Same-note stale editor text therefore triggers another editor-session mount instead of being misreported as an
  already-mounted note.
- The exception placeholder is now reserved for settled mount failures only.
  While the coordinator is still pending a snapshot refresh or body read, the host may keep its loading affordance
  visible, but it must not simultaneously claim that the note document is already unavailable.
- `mountPending` no longer wins over a presentation-ready selected-note body.
  If the selected note's resolved source arrives before the upstream loading flag drops, the coordinator clears the
  loading state from the editor surface and proceeds with the editor-session mount instead of leaving a stale loading
  mask over already-rendered content.
- `surfaceInteractive` is derived from the mount contract instead of the raw upstream loading flag.
  Once a selected-note source is parse-mounted and a document surface is ready, the editor surface may accept focus
  even if `selectedNoteBodyLoading` is still settling after startup runtime work.
- `commandSurfaceEnabled` follows `surfaceInteractive`, not `noteMounted`.
  RAW tag-management shortcuts such as `<bold>` insertion must be available as soon as source and surface are ready,
  even while the editor session is still completing its synchronization pass.
- `mountDecisionClean` now separates transient scheduling from the underlying body/surface pending state.
  Queued mount work marks the decision dirty, and `flushMount()` marks it clean as soon as a mount plan is selected.
  The view can therefore keep the editor surface visually clean even when lower-level note-body loading flags settle
  later.
