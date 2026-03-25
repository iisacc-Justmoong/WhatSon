# `src/app/qml/view/mobile/MobileNoteCreationCoordinator.qml`

## Role
`MobileNoteCreationCoordinator.qml` is a narrow mobile-only coordination object for library note creation.

It exists to keep `MobileHierarchyPage.qml` focused on route state, gesture state, and page composition while the
create-note workflow stays in a dedicated helper.

## Responsibilities
- Dispatch explicit mobile create-note requests through `windowInteractions.createNoteFromShortcut()`.
- Resolve the writable note-mutation capability through `windowInteractions.resolveLibraryNoteCreationViewModel()`.
- Listen for `emptyNoteCreated(noteId)` from that note-mutation viewmodel.
- Hold a temporary `pendingCreatedNoteId` until the shared note-list/editor models are ready.
- Emit `openEditorRequested(noteId, index)` once the page can safely promote the created note into the editor route.

## Why It Exists
The mobile page previously mixed:
- route canonicalization
- back-swipe gesture control
- note creation dispatch
- post-create editor promotion

That coupling made generic navigation code and note mutation policy too easy to confuse. Splitting creation into this
helper keeps the route shell read-oriented and the mutation workflow explicit.

## Key Functions
- `syncNoteCreationViewModel()`: refreshes the writable note-mutation reference from the interaction controller.
- `requestCreateNote()`: sends the explicit create-note command through the shared interaction controller.
- `scheduleCreatedNoteEditorRoute(noteId)`: stores a created note id and retries promotion on the next event turn.
- `routePendingCreatedNoteToEditor()`: emits `openEditorRequested(...)` only when the routed mobile page has the
  required content viewmodel, note-list model, and page router in place.

## Collaborators
- `MainWindowInteractionController.qml`: owns shortcut policy and writable LVRS view lookup.
- `MobileHierarchyPage.qml`: consumes `openEditorRequested(...)` and bridges the coordinator into the routed mobile
  page stack.
- `LibraryNoteMutationViewModel`: emits `emptyNoteCreated(noteId)` after file creation succeeds.
