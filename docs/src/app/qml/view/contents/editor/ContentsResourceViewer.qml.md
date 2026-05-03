# `src/app/qml/view/contents/editor/ContentsResourceViewer.qml`

## Responsibility

Provides the low-level resource viewport used by the dedicated resource editor.

## Current Contract

- Receives `resourceEntry`.
- Uses `ResourceBitmapViewer` to derive bitmap viewer state.
- Renders compatible bitmap resources through `Image`.
- Adds no metadata card, explanatory copy, or fallback chrome.

The current source is intentionally bitmap-only. Unsupported resource formats remain visually empty until their viewer
contracts are promoted into this component.
