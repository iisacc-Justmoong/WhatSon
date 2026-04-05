# `src/app/viewmodel/detailPanel/DetailFileStatViewModel.cpp`

## Responsibility
This implementation translates `WhatSonNoteHeaderStore` into two parallel view surfaces:
- numeric counters for direct bindings
- Figma-shaped text lines for the `description`-styled statistics panel

## Figma Text Rules
- `summaryLines` merges persisted header metadata and derived folder/tag totals:
  - project
  - total folders
  - folders list
  - total tags
  - tags list
  - created-at
  - modified-at
- `textMetricLines` exposes the textual count rows exactly in the singular labels used by Figma:
  - `Letter`
  - `Word`
  - `Sentence`
  - `Paragraph`
  - `Space`
  - `Indent`
  - `Line`
- `activityLines` exposes:
  - `Open count`
  - `Modified count`
  - `Backlink to`
  - `Backlink by`
  - `Include resources`
- `Modified count` is intentionally backed by persisted note-header update transactions, not by every debounced editor
  autosave write. Body autosave still refreshes `modified-at`, but the counter is reserved for writes that opt in to the
  modification statistic.

## Numeric Grouping Rules
- `overviewItems`
  - `totalFolders`
  - `totalTags`
  - `openCount`
  - `modifiedCount`
- `textItems`
  - `letterCount`
  - `wordCount`
  - `sentenceCount`
  - `paragraphCount`
  - `spaceCount`
  - `indentCount`
  - `lineCount`
- `relationItems`
  - `backlinkToCount`
  - `backlinkByCount`
  - `includedResourceCount`

## QML Payload Shape
Each metric row is exported as a compact map with:
- `key`
- `label`
- `value`

The numeric grouped payloads remain available for future reuse even though the current QML surface now follows the
plain-text Figma layout.

## Tests

- Automated test files are not currently present in this repository.
- Regression checklist:
  - typing in the editor and waiting for autosave must not continuously increase `modifiedCount`
  - explicit persisted metadata writes that still opt in must remain visible through `modifiedCount`
