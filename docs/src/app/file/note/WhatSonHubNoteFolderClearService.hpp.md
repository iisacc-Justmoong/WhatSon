# `src/app/models/file/note/WhatSonHubNoteFolderClearService.hpp`

## Responsibility

This header declares the service that removes all folder bindings from one indexed note.

## Collaborators

- The service resolves note IDs and header paths through `WhatSonHubNoteMutationSupport`.
- It reads and writes `.wsnhead` data through `WhatSonNoteFolderBindingRepository`.
- It returns the updated `LibraryNoteRecord` vector so the caller can refresh runtime state.

## Mutation Contract

Clearing folders is defined as writing an explicitly empty `<folders>` array. The service does not
delete the note, rewrite the body, or mutate unrelated header metadata.
