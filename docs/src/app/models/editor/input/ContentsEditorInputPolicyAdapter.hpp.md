# `src/app/models/editor/input/ContentsEditorInputPolicyAdapter.hpp`

## Responsibility

Declares the C++ editor input-policy adapter exposed to QML through `WhatSon.App.Internal`.

## Contract

- Owns native input, command-surface, shortcut, context-menu, and programmatic text-sync policy decisions.
- Exposes narrow properties and invokables only; view QML must consume the policy instead of reimplementing it.
- Exposes the standard shortcut receive contract: native modifiers are converted to standard modifiers with
  `ControlModifier` as the internal primary shortcut bit, then platform shortcut sequences are generated back to
  Meta on macOS/iOS and Control on Windows/Linux/other desktop platforms.
- Replaces the removed QML input-policy helper surface.
