# `src/app/file/hub/WhatSonHubCreator.cpp`

## Role
Implements the on-disk scaffold for a brand new `.wshub` package.

## Scaffold Output
- Creates the package directory and mandatory roots under `.whatson`, `.wscontents`, and `.wsresources`.
- Writes `.whatson/hub.json` as the primary package manifest.
- Writes the initial stat, library index, tags, folders, bookmarks, progress, and project list files.

## Behavior Notes
- Hub names are sanitized before any path is materialized.
- File writes go through `QSaveFile` for local paths, so manifest and scaffold updates stay atomic on supported filesystems.

## Tests
- `tests/app/test_whatson_workspace_hub_creator.cpp` verifies manifest creation, sanitized package layout creation, and explicit-path creation behavior.
