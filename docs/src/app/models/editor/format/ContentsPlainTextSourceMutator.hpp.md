# `src/app/models/editor/format/ContentsPlainTextSourceMutator.hpp`

## Responsibility

Declares the plain-text RAW source mutator used by ordinary typing paths.

## Public Contract

- `applyPlainTextReplacementToSource(sourceText, sourceStart, sourceEnd, replacementText)`
  Replaces one RAW source span with escaped plain text and returns the next `.wsnbody` snapshot.

## Boundary

This object is write-side only. It is not a renderer and does not expose preview/editor HTML.
