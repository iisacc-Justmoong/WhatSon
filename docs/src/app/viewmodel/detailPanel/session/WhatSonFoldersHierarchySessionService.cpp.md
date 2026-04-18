# `src/app/viewmodel/detailPanel/session/WhatSonFoldersHierarchySessionService.cpp`

## Responsibility
- Resolves `Folders.wsfolders` from the active note directory.
- Ensures that a requested folder path exists in the persisted folder hierarchy before the detail panel binds that
  folder to the current note header.

## Escaped Segment Semantics
- Folder creation and lookup now build cumulative paths through the shared
  `WhatSon::NoteFolders::appendFolderPathSegment(...)` helper.
- A literal `/` inside one folder label therefore stays escaped as `\/` in persisted ids instead of being re-expanded
  into accidental parent/child hierarchy levels during detail-panel writes.
- Existing escaped folder entries are matched by their canonical encoded path, so reassigning a note to
  `Marketing\/Sales` reuses the same folder uuid instead of creating duplicate hierarchy rows.

## Tests
- The maintained C++ regression suite now includes a session-service case that locks this escaped-slash reuse path
  against regression.
