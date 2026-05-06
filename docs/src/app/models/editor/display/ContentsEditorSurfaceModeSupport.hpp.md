# `src/app/models/editor/display/ContentsEditorSurfaceModeSupport.hpp`

## Responsibility

Declares the C++ support object that resolves the active content surface mode from the current list model.

## Contract

- Exposes `noteListModel`, `resourceEditorVisible`, and `currentResourceEntry` for QML binding.
- Keeps resource-editor routing policy in C++ instead of importing JS from the model tree.
- Emits `surfaceModeChanged` when the model or the model's resource entry changes.
