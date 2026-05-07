# `src/app/models/editor/resource/ContentsResourceImportController.hpp`

## Responsibility

Declares the C++ resource import coordinator mounted by the editor host.

## Current Contract

- Delegates drop parsing, resource-tag insertion, surface guarding, and conflict prompts to focused child controllers.
- Does not own inline resource presentation. Editor display now obtains resource visual block records from
  `ContentsEditorDisplayBackend`/`ContentsInlineResourcePresentationController`, keeping import orchestration separate
  from visual-surface rendering.
- Exposes `editorInputPolicyAdapter` so resource-import surface restores can respect the same native-input policy as
  the inline editor.
- Emits `editorInputPolicyAdapterChanged()` when that adapter binding changes.
