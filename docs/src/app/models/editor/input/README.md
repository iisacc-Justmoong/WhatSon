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
  Owns callout text snapshots, cursor geometry, selection cleanup, committed text emission, and the explicit
  plain-Enter callout-exit tag command. It also owns empty-callout Backspace deletion. Shift+Enter and other unhandled
  Enter/Backspace variants remain native callout body text input.
- `ContentsDocumentBlockController.qml`
  Owns generic document-block delegation, atomic-block tag-management keys, and mounted delegate signal forwarding.
- `ContentsDocumentTextBlockController.qml`
  Owns structured text-block live snapshots, direct plain-text RAW block mutation, inline-tag-aware source replacement
  for styled blocks, cursor geometry, and focus handling.
- `ContentsEditorInputPolicyAdapter.qml`
  Centralizes native input, shortcut-surface, context-menu, and focus-restore gating.
- `ContentsEditorSelectionController.qml`
  Resolves editor selections and routes inline style/context-menu commands.
- `ContentsEditorTypingController.qml`
  Keeps legacy whole-editor text mutation helpers and editor authoring shortcuts that still need whole-document RAW
  cursor/selection context. Generated body-tag insertion and selected-range callout wrapping payloads are delegated to
  `src/app/models/editor/tags/ContentsEditorBodyTagInsertionPlanner.*`.
- `ContentsInlineFormatEditorController.qml`
  Owns the plain-text wrapper's native input policy, selection cache, and text-edited dispatch state.

## Boundary
- Ordinary text input must continue to stay on native Qt/OS `TextEdit` handling.
- Helpers here may coordinate explicit tag-management commands or RAW mutation plans, but they must not become generic
  key overrides for ordinary note editing.
- Visual editor QML under `src/app/qml/view/content/editor` may expose wrapper properties and signals, but live typing,
  cursor bookkeeping, selection cache, source replacement, and atomic tag-management key decisions belong to these
  controller objects.
