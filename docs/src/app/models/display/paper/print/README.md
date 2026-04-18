# `src/app/models/display/paper/print`

## Responsibility
Owns print-specific paper layout and option helpers.

## Child Files
- `ContentsPagePrintLayoutRenderer.cpp`
- `ContentsPagePrintLayoutRenderer.hpp`

## Current Notes
- `ContentsPagePrintLayoutRenderer` centralizes page/print mode gating and print margin-guide calculations for every
  surface that needs paper-layout state.
- Canonical A4 geometry and paper background tokens are sourced from `src/app/models/display/paper/ContentsA4PaperBackground.*`
  so `print/` keeps the print-specific layout policy rather than owning the shared paper definition.
