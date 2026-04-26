# `src/app/models/editor/input`

## Responsibility
Owns editor input-policy and mutation-controller primitives that are not themselves visual surfaces.

## Current Modules
- `ContentsEditorInputPolicyAdapter.qml`
  Centralizes native input, shortcut-surface, context-menu, and focus-restore gating.
- `ContentsEditorSelectionController.qml`
  Resolves editor selections and routes inline style/context-menu commands.
- `ContentsEditorTypingController.qml`
  Converts committed text edits and editor authoring shortcuts into RAW `.wsnbody` mutations.

## Boundary
- Ordinary text input must continue to stay on native Qt/OS `TextEdit` handling.
- Helpers here may coordinate explicit tag-management commands or RAW mutation plans, but they must not become generic
  key overrides for ordinary note editing.
