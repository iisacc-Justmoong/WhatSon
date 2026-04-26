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
  The coordinator now keeps `noteMounted` and `commandSurfaceEnabled` false until the bound editor session is
  synchronized to that selected-note source, even when the note id already matches.
  Same-note stale editor text therefore triggers another editor-session mount instead of being misreported as an
  already-mounted note.
- The exception placeholder is now reserved for settled mount failures only.
  While the coordinator is still pending a snapshot refresh or body read, the host may keep its loading affordance
  visible, but it must not simultaneously claim that the note document is already unavailable.
