# `src/app/models/editor/display/ContentsEditorSurfaceModeSupport.cpp`

## Responsibility

Implements content-surface mode resolution for note-backed and resource-backed list models.

## Current Behavior

- A resource editor is visible only when the active list model is not note-backed and exposes a non-empty
  `currentResourceEntry`.
- The current resource entry is returned as a `QVariantMap`; note-backed or missing models return an empty map.
- The object connects to model notify signals when available so view bindings update without JS helper imports.
