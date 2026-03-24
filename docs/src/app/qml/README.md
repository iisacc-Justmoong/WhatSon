# `src/app/qml`

## Role
This directory contains the root LVRS application shell and the top-level QML-side interaction controllers.

The core rule in this directory is that visual composition belongs here, while persistence and most mutation logic should stay in C++ viewmodels or in narrow bridge/helper objects.

## Important Files
- `Main.qml`: the root `LV.ApplicationWindow`, route shell, and root `LV.ViewModels` registration point.
- `MainWindowInteractionController.qml`: shortcut, resize-policy, and focus-management helper bound to the root window.
- `DesignTokens.qml`: QML-side design token aggregation.

## Ownership Model
The C++ composition root still injects bootstrap objects through context properties. `Main.qml` then re-registers the important viewmodels into `LV.ViewModels` and claims write ownership for selected view IDs. This split keeps startup compatibility while moving mutation rights toward explicit LVRS ownership.

## Why This Directory Is Important
If a runtime object exists in C++ but behaves incorrectly in the UI, this directory is usually where the mismatch becomes visible first.
