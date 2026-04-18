# `src/app/models/display/paper/ContentsA4PaperBackground.cpp`

## Responsibility

Implements the canonical A4 paper background tokens shared by page and print display models.

## Key Behavior

- Publishes the physical A4 portrait size in millimeters:
  - width: `210`
  - height: `297`
  - aspect ratio: `210 / 297`
- Publishes the fixed paper kind as the shared `ContentsPaperSelection::A4` enum value.
- Publishes the paper-preview palette used by the editor's page/print surfaces:
  - canvas color
  - paper fill and highlight
  - border, separator, and shadow colors
  - paper text color
- Exposes `requestRefresh()` as the explicit hook slot so QML can force a notifier turn when bindings need to
  rebroadcast the immutable background state.

## Regression Checks

- The background object must keep the canonical A4 geometry in millimeters instead of introducing a second ratio-only
  source of truth.
- The background object must identify itself as `ContentsPaperSelection::A4`.
- `ContentsPagePrintLayoutRenderer` must consume the same default aspect ratio and color tokens so the print layout
  path stays visually aligned with the common paper domain.
