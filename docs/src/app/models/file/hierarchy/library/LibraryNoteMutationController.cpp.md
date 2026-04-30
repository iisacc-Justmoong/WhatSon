# `src/app/models/file/hierarchy/library/LibraryNoteMutationController.cpp`

## Implementation Notes
- The mutation facade now discovers `ILibraryNoteMutationCapability` through `qobject_cast`.
- Mutation signals are forwarded through QObject signal wiring instead of compile-time concrete-type coupling.
- Batch deletion and folder-clearing helpers normalize and deduplicate incoming note ids before replaying the existing
  single-note capability calls.
- This keeps QML batch note-list actions in one facade instead of scattering per-id mutation loops through
  `ListBarLayout.qml`.
