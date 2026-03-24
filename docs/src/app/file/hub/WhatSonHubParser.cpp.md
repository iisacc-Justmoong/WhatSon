# `src/app/file/hub/WhatSonHubParser.cpp`

## Responsibility

This file is the bootstrap parser for a `.wshub` package. It reads persisted hub files and produces
the in-memory runtime payload consumed by higher-level stores and viewmodels.

## Folder Hierarchy Output

For folder hierarchies, the parser now forwards the full `WhatSonFolderDepthEntry` contract,
including the stable `uuid` field. That makes UUID identity available immediately during startup,
before the library hierarchy viewmodel or mutation services start filtering notes.

## Compatibility Notes

- Legacy hub packages without folder UUIDs are still accepted because the lower-level folder parser
  upgrades them.
- New hub packages preserve UUIDs end to end, so a folder rename performed in one session remains a
  rename rather than a delete-and-recreate event in the next session.
