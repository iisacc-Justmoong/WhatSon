# `ContentsDisplayViewportCoordinator.hpp`

- Declares the editor viewport/minimap coordinator used by `ContentsDisplayView.qml`.
- The public invokables now cover line-offset lookup and minimap viewport math so QML no longer needs to own those
  binary-search and proportional-layout calculations directly.
- `minimapLineBarWidth(...)` maps measured editor line width into minimap track width, with the legacy character-count
  width kept only as a fallback when no sampled line width is available.
