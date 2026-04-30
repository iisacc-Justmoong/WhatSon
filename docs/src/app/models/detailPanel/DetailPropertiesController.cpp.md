# `src/app/models/detailPanel/DetailPropertiesController.cpp`

## Responsibility
- Projects note-header metadata into the detail panel's properties section.
- Keeps folder/tag item lists plus the current project, bookmark, and progress values in one lightweight controller.

## Folder Presentation Rules
- Folder entries prefer `leafFolderName(...)` for compact chip-style presentation.
- When the persisted folder path has only one logical segment, the fallback now uses
  `displayFolderPath(...)` instead of the raw stored value.
- This prevents escaped persistence markers such as `\/` from leaking into the detail panel when one folder label
  literally contains `/`.

## Tests
- The maintained C++ regression suite locks the shared folder-path escaping semantics that this view depends on.
