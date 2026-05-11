# `src/app/models/navigationbar/EditorViewSectionController.hpp`

## Responsibility
Declares the QObject section state exposed for one editor view-mode entry.

## Public Contract
- `active`: whether this section is the current combo selection.
- `editorViewValue`: stable integer value consumed by QML menu state.
- `editorViewName`: visible menu label.
- `requestControllerHook()`: standard controller hook signal used by panel tests and diagnostics.
