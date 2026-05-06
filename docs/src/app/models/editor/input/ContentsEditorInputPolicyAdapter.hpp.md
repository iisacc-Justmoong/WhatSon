# `src/app/models/editor/input/ContentsEditorInputPolicyAdapter.hpp`

## Responsibility

Declares the C++ editor input-policy adapter exposed to QML through `WhatSon.App.Internal`.

## Contract

- Owns native input, command-surface, shortcut, context-menu, and programmatic text-sync policy decisions.
- Exposes narrow properties and invokables only; view QML must consume the policy instead of reimplementing it.
- Replaces the removed QML input-policy helper surface.
