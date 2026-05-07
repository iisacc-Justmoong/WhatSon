# `src/app/models/editor/input/ContentsWysiwygEditorPolicy.hpp`

## Responsibility

Declares the C++ policy object exposed to QML as `ContentsWysiwygEditorPolicy`.

## Contract

- Owns WYSIWYG editor calculations that depend on RAW `.wsnbody` offsets and rendered logical offsets.
- Keeps visible-text mutation planning, hidden inline tag cursor normalization, visible-surface selection mapping,
  hidden-wrapper selection trimming, line/paragraph logical range selection, atomic resource-block selection policy,
  and atomic resource-block cursor exclusion policy out of QML.
- Treats `.wsnbody` source offsets as the authoritative write-side coordinates; rendered text coordinates are only
  read-side projection coordinates supplied by `ContentsEditorPresentationProjection`.
- Exposes pure `Q_INVOKABLE` helpers for the view. The view still applies TextEdit focus, geometry, and selection
  changes because those are visual-surface operations.
