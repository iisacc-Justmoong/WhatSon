# `src/app/models/display/paper/ContentsPaperSelection.hpp`

## Responsibility

Declares the canonical paper-selection enum object used to describe which paper standard the user chose.

## Public Contract

- Selection state
  - `selectedPaperKind`
  - `selectedPaperStandard`
- Enum values
  - `Unknown`
  - `A4`
  - `Letter`
  - `Legal`
  - `A5`
  - `B5`
- Helpers
  - `paperStandardForKind(...)`
  - `normalizePaperKind(...)`
  - `paperStandardForValue(...)`
  - `setSelectedPaperKindByValue(...)`
  - `requestRefresh()`

## Signals

- `selectedPaperKindChanged`

## Notes

- The object is intentionally placed in `models/display/paper` so every paper-aware surface can share one enum source
  instead of duplicating paper identifiers in page-only or print-only code.
- The default selection remains `A4`, matching the current paper background and print layout defaults, while still
  leaving room for future non-A4 options.
