# `src/app/models/display`

## Responsibility
Owns display-mode model helpers that sit below QML and above raw editor/controller consumers.

## Current Domains
- `paper`: canonical A4 paper background definitions plus common paper-surface presentation helpers shared by page and
  print view modes, including the shared paper-selection enum object.

## Architectural Note
- Display-mode objects belong under `models` because they expose stable QObject state and calculations for the UI, but
  they are not themselves QML views and should not live in the editor renderer tree once they become reusable
  cross-surface mode helpers.
