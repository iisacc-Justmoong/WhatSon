# `src/app/viewmodel/hierarchy/library/LibraryNoteMutationViewModel.cpp`

## Implementation Notes
- The mutation facade now discovers `ILibraryNoteMutationCapability` through `qobject_cast`.
- Mutation signals are forwarded through QObject signal wiring instead of compile-time concrete-type coupling.
