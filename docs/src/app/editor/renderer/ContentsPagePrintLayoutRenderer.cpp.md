# `src/app/editor/renderer/ContentsPagePrintLayoutRenderer.cpp`

## Responsibility

Implements page/print editor rendering state for paper-preview surfaces.

## Key Behavior

- Maps editor mode integers to Page/Print behavior using `EditorViewState` values instead of QML-local constants.
- Derives Page/Print visibility state from three runtime facts:
  - active editor view mode
  - note-selection presence
  - dedicated resource-viewer visibility
- Calculates paper geometry in C++:
  - resolved paper width/height from viewport and A4 ratio
  - text content rectangle size from guide insets
  - page count from current editor content height
  - document surface height from paper stack + viewport clamp
- Normalizes incoming numeric values:
  - non-finite or negative dimensions become safe non-negative values
  - invalid ratio/thickness input falls back to safe defaults
- Provides centralized paper visual tokens (canvas/paper/border/shadow/separator/text colors) for both desktop and mobile
  editor surfaces.
- Exposes explicit `requestRefresh()` hook to force mode/layout notifier emission when QML needs a manual rebinding turn.

## Regression Checks

- `Page` mode must show paper layout even when `Print` mode is inactive.
- `Print` mode must show paper layout plus margin guides.
- Non-plain modes must collapse back to non-paper layout when no note is selected.
- Dedicated resource-viewer notes must suppress page/print paper layout.
- Paper width must remain bounded by viewport minus horizontal margins and `paperMaxWidth`.
- Page count must stay at least `1` and must grow when editor content height exceeds one page text height.
- Desktop and mobile surfaces must consume the same backend-derived paper metrics and color tokens.
