# `ContentsDisplayViewportCoordinator.cpp`

- Implements editor viewport correction plans, structured gutter signatures, logical-line offset helpers, and minimap
  proportional geometry.
- Minimap row width now has a measured-line path: when a row carries `contentWidth` and `contentAvailableWidth`, the
  coordinator scales that real editor silhouette into the minimap track and uses character-count width only as a legacy
  fallback.
- This file is the main C++ sink for line/minimap calculations that previously lived inline in
  `ContentsDisplayView.qml`.
