# `src/app/models/editor/display/ContentsDisplayMinimapCoordinator.cpp`

## Responsibility

Implements minimap snapshot normalization and row construction for the editor display host.

## Current Behavior

- Structured minimap snapshots now come from normalized `.wsnbody` block entries instead of structured logical-line
  geometry.
- One parser-normalized block/tag becomes one minimap line group.
- Text-like blocks derive bar length from the amount of plain text they contain.
- Resource blocks keep `minimapVisualKind = block` and receive a near-full-width silhouette.
- The structured minimap still scales its synthetic rows across the current document content height so viewport-thumb
  math and click-scroll proportionality stay aligned with the live editor flickable.
- Previous-snapshot token comparison and partial-splice planning are intentionally absent; the display host rebuilds
  minimap groups from the current document state on each queued refresh.

## Boundary

This file owns structured minimap simplification only.
Gutter logical-line geometry and cursor-row measurement still belong to the structured flow and viewport coordinators.
