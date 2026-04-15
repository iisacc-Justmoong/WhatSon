# `src/app/qml/view/content/editor/ContentsDocumentTextBlock.qml`

## Responsibility
Edits one ordinary structured text block while keeping RAW `.wsnbody` source authoritative and projecting inline style
tags as visible formatted text.

## Current Behavior
- The block now binds its RAW inner block source into `ContentsTextFormatRenderer.editorSurfaceHtml` and feeds that
  HTML projection into `ContentsInlineFormatEditor.qml` as a `TextEdit.RichText` editing surface.
- Inline tags such as `<bold>`, `<italic>`, `<underline>`, `<strikethrough>`, and `<highlight>` therefore no longer
  appear literally in the visible editor surface after a formatting command.
- Live typing is still source-driven:
  - compare previous visible plain text with current editor plain text
  - compute the changed logical range
  - map that logical range back into inline-tag-aware source offsets through
    `ContentsStructuredCursorSupport.js`
  - rewrite the RAW block source through `ContentsTextFormatRenderer.applyPlainTextReplacementToSource(...)`
- Inline-format shortcuts no longer wrap raw text with local string surgery inside this QML block.
  They now call `ContentsTextFormatRenderer.applyInlineStyleToLogicalSelectionSource(...)` so selection-based style
  rewrites preserve the existing inline-style coverage model.
- Focus restoration and caret-origin source offsets are now mapped through the same inline-tag-aware cursor bridge.
  Rich text cursor positions stay in visible plain-text space, while reparsed RAW offsets still return to the same
  visible caret location.
- Gutter/minimap line layout still follows the live editor surface geometry via `positionToRectangle(...)`, but the
  logical line content now comes from the visible plain-text projection rather than the literal RAW tag string.
- The block still emits only RAW mutation requests upward; the rich-text surface remains a read-side/editor-side
  projection, not a persistence authority.

## Shared Block Contract
- `textEditable = true`
- `atomicBlock = false`
- `gutterCollapsed = false`
- `minimapVisualKind = text`
- `visiblePlainText()` returns the current visible plain-text editor content
- `representativeCharCount(...)` follows the visible line text length

## Architecture Note
- For semantic wrapper blocks such as `<paragraph>...</paragraph>`, the parser still passes only the inner editable
  content into `sourceText` / `sourceStart` / `sourceEnd`.
- This component therefore edits wrapper content directly without rendering or mutating the outer wrapper tags.
