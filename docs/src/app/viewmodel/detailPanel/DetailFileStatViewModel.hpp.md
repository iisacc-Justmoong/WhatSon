# `src/app/viewmodel/detailPanel/DetailFileStatViewModel.hpp`

## Responsibility
`DetailFileStatViewModel` is the dedicated detail-panel statistics object for the `fileStat` toolbar state.
It mirrors the numeric `.wsnhead <fileStat>` fields into QML-friendly properties and also exposes the Figma-shaped
text blocks used by the statistics view.

## Exported Metric Properties
- `totalFolders`
- `totalTags`
- `letterCount`
- `wordCount`
- `sentenceCount`
- `paragraphCount`
- `spaceCount`
- `indentCount`
- `lineCount`
- `openCount`
- `modifiedCount`
- `backlinkToCount`
- `backlinkByCount`
- `includedResourceCount`

## Figma Text Surface
- `summaryLines`
  - `Projects: ...`
  - `Total folders: ...`
  - `Folders: ...`
  - `Total tags: ...`
  - `Tags: ...`
  - `Created at: ...`
  - `Modified at: ...`
- `textMetricLines`
  - `Letter: ...`
  - `Word: ...`
  - `Sentence: ...`
  - `Paragraph: ...`
  - `Space: ...`
  - `Indent: ...`
  - `Line: ...`
- `activityLines`
  - `Open count: ...`
  - `Modified count: ...`
  - `Backlink to: ...`
  - `Backlink by: ...`
  - `Include resources: ...`

## Grouped QML Surface
- `overviewItems`: folders, tags, opens, modified
- `textItems`: letters, words, sentences, paragraphs, spaces, indents, lines
- `relationItems`: linked-to, linked-by, resources

## Lifecycle
- `applyHeader(...)` copies the currently loaded note header snapshot into the viewmodel.
- `clearHeader()` resets the surface back to the zero-state contract used by empty or unresolved notes.
- Every update emits `statsChanged()`, so QML can refresh both the numeric properties and the text blocks from the same
  source object.
