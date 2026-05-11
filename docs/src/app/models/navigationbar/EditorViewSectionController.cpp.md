# `src/app/models/navigationbar/EditorViewSectionController.cpp`

## Responsibility
Stores one editor view-mode entry and emits `activeChanged()` only when the active flag actually changes.

## Runtime Role
Instances are owned by `EditorViewModeController`; QML receives them as read-only section controllers for selected-item
presentation.
