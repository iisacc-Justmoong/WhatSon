# `src/app/models/editor/display/ContentsDisplayMinimapCoordinator.hpp`

## Responsibility

Declares the editor-domain minimap coordinator.

## Public Contract

- `buildStructuredMinimapSnapshotEntries(blockEntries)`
  Normalizes parser-derived structured document blocks into one minimap snapshot entry per block/tag.
- `buildStructuredMinimapLineGroupsForRange(snapshotEntries, startLineNumber, endLineNumber, documentContentHeight)`
  Builds the paintable minimap line groups for that structured snapshot range.

## Boundary

This coordinator does not sample live `TextEdit` rectangles for the structured minimap path anymore.
Its structured responsibility is now parser/block driven: one normalized block becomes one minimap line, and only
visual blocks such as resources are widened into block-like silhouettes.
Snapshot reuse planning is not part of this coordinator; the display host rebuilds the minimap rows from the current
document state on each queued minimap refresh so an initial empty note silhouette cannot outlive the rendered body.
