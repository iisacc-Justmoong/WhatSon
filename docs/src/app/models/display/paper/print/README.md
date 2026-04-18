# `src/app/models/display/paper/print`

## Responsibility
Owns print-specific paper layout and option helpers.

## Child Files
- `ContentsPagePrintLayoutRenderer.cpp`
- `ContentsPagePrintLayoutRenderer.hpp`

## Current Notes
- `ContentsPagePrintLayoutRenderer` centralizes page/print mode gating, A4 paper geometry, and print margin-guide
  calculations for every surface that needs paper-layout state.
