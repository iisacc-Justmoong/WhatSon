# `src/app/qml/view/content/editor/ContentsDocumentTextBlock.qml`

## Responsibility
Edits one ordinary structured text block while keeping RAW `.wsnbody` source authoritative and projecting inline style
tags as visible formatted text.

## Current Behavior
- The block now keeps `ContentsInlineFormatEditor.qml` in `TextEdit.PlainText` mode and uses
  `ContentsTextFormatRenderer.editorSurfaceHtml` only as the visual overlay payload.
- Inline tags such as `<bold>`, `<italic>`, `<underline>`, `<strikethrough>`, and `<highlight>` therefore no longer
  appear literally in the visible editor surface after a formatting command, but the editable buffer itself still
  stays plain text instead of a serialized Qt RichText document.
- That formatted overlay is now mounted only when the block source actually contains inline style tags.
  Ordinary plain paragraphs therefore no longer pay for one extra rich-text overlay paint path when they are just
  plain text.
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
- The block still emits only RAW mutation requests upward; the rendered overlay remains a read-side projection and
  never becomes the persistence authority.
- The nested inline editor now also runs one host-owned shortcut handler before its local boundary-navigation logic.
  Note-wide shortcuts such as clipboard-image paste can therefore be intercepted while focus is inside a structured
  paragraph editor, without reintroducing legacy whole-note editing authority.
- An empty text block now treats plain `Backspace` / `Delete` as "remove this line" before it tries adjacent atomic
  block deletion.
  Zero-length paragraph blocks therefore no longer become undeletable cursor anchors.

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
