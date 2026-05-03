# `src/app/models/editor/resource/ContentsResourceImportController.hpp`

## Responsibility

Declares the C++ resource import coordinator mounted by the editor host.

## Current Contract

- Delegates drop parsing, resource-tag insertion, inline resource HTML substitution, surface guarding, and conflict
  prompts to focused child controllers.
- Exposes `editorInputPolicyAdapter` so resource-import surface restores can respect the same native-input policy as
  the inline editor.
- Emits `editorInputPolicyAdapterChanged()` when that adapter binding changes.
