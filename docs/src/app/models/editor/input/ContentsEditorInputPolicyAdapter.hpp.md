# `src/app/models/editor/input/ContentsEditorInputPolicyAdapter.hpp`

## Responsibility

Declares the C++ editor input-policy adapter exposed to QML through `WhatSon.App.Internal`.

## Contract

- Owns native input, command-surface, shortcut, context-menu, and programmatic text-sync policy decisions.
- Exposes narrow properties and invokables only; view QML must consume the policy instead of reimplementing it.
- Exposes the standard shortcut receive contract: native modifiers are converted to standard modifiers with
  `ControlModifier` as the internal primary shortcut bit. Qt maps Command shortcuts to `ControlModifier` on
  macOS/iOS by default, so generated platform shortcut sequences also stay `Ctrl+...` and let Qt present/route them as
  Command shortcuts on Apple platforms.
- Exposes a text-aware tag-management shortcut request path so QML can pass `KeyEvent.text`; macOS Option-modified
  symbols such as `ç` can then be canonicalized in C++ instead of in view code.
- Replaces the removed QML input-policy helper surface.
