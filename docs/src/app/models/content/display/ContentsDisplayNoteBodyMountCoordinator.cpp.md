# `ContentsDisplayNoteBodyMountCoordinator.cpp`

- Coordinates note-open mount decisions between selected-note snapshots, the bound editor session, and the active
  document surface.
- Explicit empty-body snapshots that are resolved for the selected note are treated as valid mountable sources, so the
  editor can open a blank note without falling back to the `"Note body unavailable"` exception surface.
