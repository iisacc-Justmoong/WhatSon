# `src/app/models/editor/resource/ContentsResourceImportConflictController.hpp`

## Responsibility

Declares the C++ duplicate-resource conflict controller for editor imports.

## Contract

- Owns pending duplicate-import decision state.
- Exposes overwrite, keep-both, and cancel decisions through C++ signals and slots.
- Keeps conflict policy out of QML view files.
