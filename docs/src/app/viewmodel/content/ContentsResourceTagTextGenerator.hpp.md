# `src/app/viewmodel/content/ContentsResourceTagTextGenerator.hpp`

## Role
Declares the QML bridge that exposes canonical RAW resource-tag generation to editor-side import controllers.

## Surface
- `normalizeImportedResourceEntry(...)` returns the normalized metadata descriptor for one imported resource payload.
- `buildCanonicalResourceTag(...)` emits the final `<resource ... />` string and updates the debug-facing
  `lastGenerated*` properties.
