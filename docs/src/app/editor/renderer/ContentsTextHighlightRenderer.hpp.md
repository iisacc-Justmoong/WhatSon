# `src/app/editor/renderer/ContentsTextHighlightRenderer.hpp`

## Responsibility
Defines the dedicated highlight-style renderer contract used by text-format rendering.

## Public Contract
- `isHighlightTagAlias(elementName)`
  Returns true for `.wsnbody` inline highlight aliases (`highlight`, `mark`).
- `highlightOpenHtmlTag()`
  Returns the Apple Notes-inspired inline style opener:
  dark orange background + bright orange foreground + semibold text.
- `highlightCloseHtmlTag()`
  Returns the matching closing tag.
