# `src/app/models/editor/input`

## Responsibility
Owns editor input-policy and mutation-controller primitives that are not themselves visual surfaces.

## Current Modules
- `ContentsAgendaBlockController.qml`
  Owns aggregate agenda-block queries that do not draw the agenda row UI.
- `ContentsAgendaTaskRowController.qml`
  Owns agenda task row focus, live text snapshots, toggle forwarding, cursor geometry, and committed text emission.
- `ContentsBreakBlockController.qml`
  Owns break-block tag-management key handling and selection activation.
- `ContentsCalloutBlockController.qml`
  Owns callout text snapshots, cursor geometry, selection cleanup, and committed text emission.
- `ContentsDocumentBlockController.qml`
  Owns generic document-block delegation, atomic-block tag-management keys, and mounted delegate signal forwarding.
- `ContentsDocumentTextBlockController.qml`
  Owns structured text-block live snapshots, inline-tag-aware source replacement, cursor geometry, and focus handling.
- `ContentsEditorInputPolicyAdapter.qml`
  Centralizes native input, shortcut-surface, context-menu, and focus-restore gating.
- `ContentsEditorSelectionController.qml`
  Resolves editor selections and routes inline style/context-menu commands.
- `ContentsEditorTypingController.qml`
  Converts committed text edits and editor authoring shortcuts into RAW `.wsnbody` mutations.
- `ContentsInlineFormatEditorController.qml`
  Owns the plain-text wrapper's native input policy, selection cache, macOS Option-word navigation, and text-edited
  dispatch state.

## Boundary
- Ordinary text input must continue to stay on native Qt/OS `TextEdit` handling.
- Helpers here may coordinate explicit tag-management commands or RAW mutation plans, but they must not become generic
  key overrides for ordinary note editing.
- Visual editor QML under `src/app/qml/view/content/editor` may expose wrapper properties and signals, but live typing,
  cursor bookkeeping, selection cache, source replacement, and atomic tag-management key decisions belong to these
  controller objects.
