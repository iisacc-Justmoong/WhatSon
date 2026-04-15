# `src/app/qml/view/content/editor/ContentsDocumentBlock.qml`

## Responsibility
Provides the single document-block adapter that keeps `ContentsStructuredDocumentFlow.qml` from depending directly on
 block-specific delegates.

## Current Behavior
- The flow now instantiates this component for every structured row.
- An internal `Loader` chooses the concrete implementation from `blockData.type`:
  - `ContentsDocumentTextBlock.qml`
  - `ContentsAgendaBlock.qml`
  - `ContentsCalloutBlock.qml`
  - `ContentsResourceBlock.qml`
  - `ContentsBreakBlock.qml`
- The outer API deliberately stays generic:
  - focus / current-line / cursor-row geometry queries
  - delete-key forwarding
  - host-owned shortcut forwarding
  - inline-format selection snapshot + apply helpers
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
- The adapter now also forwards one host-provided shortcut handler into both text-like delegates and atomic-block key
  handling.
  Note-wide shortcuts such as clipboard-image paste therefore keep working even when focus currently sits inside one
  structured paragraph editor or on a selected break/resource block.
- `Connections { ignoreUnknownSignals: true }` re-emits only the signals that the active inner block actually exposes.

## Architecture Note
- The goal of this file is not to eliminate every internal block-type branch.
- Its role is to isolate that branch behind one adapter layer so the flow host consumes a document-block contract
  instead of assembling several specialized editor widgets directly.
- In other words, block type still chooses presentation, but editability/focus/gutter/minimap policy is expected to
  flow through the shared block contract rather than through type-specific host logic.
