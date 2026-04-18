# `src/app/models/display/paper`

## Responsibility
Owns paper-surface helpers shared by page and print editor modes.

## Child Files
- `ContentsA4PaperBackground.cpp`
- `ContentsA4PaperBackground.hpp`
- `ContentsTextFormatRenderer.cpp`
- `ContentsTextFormatRenderer.hpp`

## Child Directories
- `print`

## Current Notes
- `ContentsA4PaperBackground` is the canonical A4 paper background object for shared paper geometry and palette tokens.
- `ContentsTextFormatRenderer` remains the canonical RAW-to-HTML text projection backend, but its page/print paper
  palette responsibilities now live under the paper display model domain instead of the editor renderer domain.
