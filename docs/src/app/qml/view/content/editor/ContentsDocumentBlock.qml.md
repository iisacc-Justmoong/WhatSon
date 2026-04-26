# `src/app/qml/view/content/editor/ContentsDocumentBlock.qml`

## Responsibility
Provides the single document-block adapter that keeps `ContentsStructuredDocumentFlow.qml` from depending directly on
 block-specific delegates.

## Current Behavior
- Non-visual adapter behavior now lives in `src/app/models/editor/input/ContentsDocumentBlockController.qml`.
  This view keeps the generic document-block layout and delegate loader; atomic tag-management key handling, mounted
  delegate signal forwarding, selection-clear propagation, and focus routing belong to the controller.
- The flow now instantiates this component for every structured row.
- A structured row is now either one parsed explicit block or one flattened interactive prose group assembled by
  `ContentsStructuredDocumentFlow.qml`.
- An internal `Loader` chooses the concrete implementation from `blockData.type`:
  - `ContentsDocumentTextBlock.qml`
  - `ContentsAgendaBlock.qml`
  - `ContentsCalloutBlock.qml`
  - `ContentsResourceBlock.qml`
  - `ContentsBreakBlock.qml`
- The outer API deliberately stays generic:
  - focus / current-line / cursor-row geometry queries
  - tag-management delete handling for selected atomic resource/break blocks
  - host-owned tag-management shortcut forwarding for selected atomic blocks
  - inline-format selection snapshot export for the flow-level formatting controller
  - shortcut insertion offset lookup
  - generic block interaction signals
- The adapter now also proxies the parser/delegate block contract back to the flow host:
  - `textEditable`
  - `atomicBlock`
  - `gutterCollapsed`
  - `minimapVisualKind`
  - `minimapRepresentativeCharCount`
  - `visiblePlainText()`
  - `representativeCharCount(...)`
- Atomic blocks are now also focused and navigated at this adapter layer.
  `ContentsDocumentBlock.qml` owns block selection, arrow-key traversal, delete handling, and Enter-to-resume-edit
  behavior for non-text resource/break spans instead of delegating that behavior to a resource-local focus widget.
- The adapter now also exposes mounted delegates' native composition state (`inputMethodComposing` and `preeditText`) to
  the structured flow.
  Window-level shortcuts can therefore stand down while a nested editor is in IME composition.
- The adapter keeps host-provided tag-management shortcut forwarding limited to selected atomic-block key handling.
  Text-like delegates no longer receive QML shortcut handlers on top of their live `TextEdit`.
- Atomic resource/break blocks now leave modified OS text-navigation chords to the platform path.
  The adapter no longer branches on `AltModifier` at all. Only plain atomic-block navigation/deletion and exact
  Command Up/Down document-boundary movement remain host-owned, so macOS Option-arrow input cannot be consumed by the
  atomic-block adapter.
- The adapter now also listens to the structured document host's selection-clear revision and forwards
  `clearSelection(preserveFocusedEditor)` into the mounted delegate.
  Selection cleanup therefore stays centralized at the host boundary instead of being reimplemented for every viewport
  click path.
- Inline-format RAW rewrites are now intentionally absent from this adapter surface.
  The mounted block still reports live selection state, but the owning structured editor controller applies style
  commands against the RAW source span at the flow layer.
- `Connections { ignoreUnknownSignals: true }` re-emits only the signals that the active inner block actually exposes.

## Architecture Note
- The goal of this file is not to eliminate every internal block-type branch.
- Its role is to isolate that branch behind one adapter layer so the flow host consumes a document-block contract
  instead of assembling several specialized editor widgets directly.
- In other words, block type still chooses presentation, but editability/focus/gutter/minimap policy is expected to
  flow through the shared block contract rather than through type-specific host logic.
