# `src/app/models/content/ContentsStructuredDocumentHost.cpp`

## Responsibility
Implements the structured document host state container and the small state transitions that QML depends on for focus,
layout cache, and selection cleanup.

## Current Behavior
- Emits change signals only when the stored value actually changes.
- Keeps structured-flow diagnostics visible through `WhatSon::Debug::traceEditorSelf(...)`.
- `noteActiveBlockInteraction(...)` handles focus/activation transitions:
  - advances the selection-clear revision while retaining the active block
  - stores the active block index
- `noteActiveBlockCursorInteraction(...)` handles live cursor movement:
  - clears stale selection only if the cursor event belongs to a different active block
  - always bumps the active-block cursor revision so minimap/caret chrome follows same-block cursor moves
- `requestSelectionClear(...)` can also be called independently, allowing trailing-margin terminal body clicks to clear
  every stale structured editor selection before focus is restored elsewhere.

## Regression Focus
- The maintained C++ regression suite now locks the split between focus activation and cursor movement: same-block
  cursor movement must not clear live TextEdit selections, while focus activation and real active-block changes still
  clear stale selections in other structured editors.
