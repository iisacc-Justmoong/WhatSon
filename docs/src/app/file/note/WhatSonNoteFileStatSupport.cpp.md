# `src/app/file/note/WhatSonNoteFileStatSupport.cpp`

## Responsibility

This helper owns the derived `.wsnhead <fileStat>` rules.

## Derived Counters

- Header-derived:
  - `totalFolders`
  - `totalTags`
- Body-derived:
  - `letterCount`
  - `wordCount`
  - `sentenceCount`
  - `paragraphCount`
  - `spaceCount`
  - `indentCount`
  - `lineCount`
  - `backlinkToCount`
  - `includedResourceCount`
- Hub-derived:
  - `backlinkByCount`

## Tracking Rules

- `openCount` is incremented only through `refreshTrackedStatisticsForNote(..., true)`.
- The helper intentionally does not mutate `modifiedCount`; write paths own that counter.
- Incoming backlink counts are resolved by scanning other `.wsnbody` files under the enclosing
  `.wshub` package and skipping hidden staged-delete directories.
