# `src/app/viewmodel/content/ContentsStructuredDocumentHost.cpp`

## Responsibility
Implements the structured document host state container and the small state transitions that QML depends on for focus,
layout cache, and selection cleanup.

## Current Behavior
- Emits change signals only when the stored value actually changes.
- Keeps structured-flow diagnostics visible through `WhatSon::Debug::traceEditorSelf(...)`.
- `noteActiveBlockInteraction(...)` now performs three coupled actions in one turn:
  - advances the selection-clear revision while retaining the active block
  - stores the active block index
  - bumps the active-block cursor revision
- `requestSelectionClear(...)` can also be called independently, allowing blank-area clicks to clear every stale
  structured editor selection before focus is restored elsewhere.

## Regression Focus
- The maintained C++ regression suite now locks the selection-clear revision contract so repeated same-block
  interactions still emit a fresh cleanup turn for nested editors such as agenda tasks.
