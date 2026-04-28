# `src/app/models/editor/format/ContentsPlainTextSourceMutator.cpp`

## Responsibility

Implements ordinary plain-text RAW source replacement for editor typing paths.

## Current Behavior

- Normalizes line endings in both source and replacement text.
- Clamps source bounds against an `int`-safe `QString` length before stitching.
- Escapes inserted literal text before it is written back into `.wsnbody`.
- Canonicalizes committed or pasted web links into proprietary `<weblink ...>...</weblink>` RAW tags.

## Boundary

This file deliberately owns only write-side source replacement. Rendering and overlay projection remain in dedicated
renderer objects.
