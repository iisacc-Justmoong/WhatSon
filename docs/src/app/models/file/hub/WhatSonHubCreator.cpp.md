# `src/app/models/file/hub/WhatSonHubCreator.cpp`

## Role
Implements the on-disk scaffold for a brand new `.wshub` package after the package root has been materialized.

## Scaffold Output
- Creates the mandatory roots under `.whatson`, `.wscontents`, and `.wsresources`.
- Writes `.whatson/hub.json` as the primary package manifest.
- Writes the initial stat, library index, tags, folders, bookmarks, progress, and project list files.

## Behavior Notes
- Hub names are sanitized before any path is materialized.
- Package-root creation and file-like package presentation are delegated to `WhatSonHubPackager`.
- If scaffold creation fails after the package root exists, the creator removes the partially created package directory.
- File writes go through `QSaveFile` for local paths, so manifest and scaffold updates stay atomic on supported filesystems.

## Tests
- `test/cpp/whatson_cpp_regression_tests.cpp`
  covers the split between package materialization and scaffold writes, together with package-root rollback.
