# `src/app/qml/view/panels/detail/DetailFileStatForm.qml`

## LVRS Token Notes
- File-stat line fallback widths and section spacing use `LV.Theme.gapNone`.

## Responsibility
`DetailFileStatForm.qml` renders the real `fileStat` detail-panel surface instead of the old placeholder.

## Input Contract
- `fileStatViewModel`
  - Expected type: `DetailFileStatViewModel`
  - Consumed text collections:
    - `summaryLines`
    - `textMetricLines`
    - `activityLines`

## Visual Structure
- One narrow `Form` column with horizontal/top inset sourced from `LV.Theme.gap8` / `LV.Theme.gap2`
- Three text groups separated by `LV.Theme.gap10`
- Every rendered line uses `LV.Label { style: description }`

This matches the Figma node `235:7734`, where the statistics panel is a plain `description` typography surface rather
than a card grid.
