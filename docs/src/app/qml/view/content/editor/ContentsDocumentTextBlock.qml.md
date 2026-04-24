# `src/app/qml/view/content/editor/ContentsDocumentTextBlock.qml`

## Responsibility
Edits one ordinary structured text block while keeping RAW `.wsnbody` source authoritative and projecting inline style
tags as visible formatted text.

In the structured-flow host this delegate may now also receive one flattened interactive prose span that was assembled
from multiple implicit parser blocks.

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
- When the host passes a flattened interactive prose span, that RAW rewrite now applies to the whole grouped source
  slice instead of to only one parser paragraph entry.
- Inline-format shortcuts no longer mutate RAW from inside this delegate at all.
  The block exposes only the live selection snapshot, while
  `ContentsStructuredEditorFormattingController.qml` performs the actual selection-based RAW rewrite for the owning
  structured editor session.
- Focus restoration and caret-origin source offsets are now mapped through the same inline-tag-aware cursor bridge.
  Rich text cursor positions stay in visible plain-text space, while reparsed RAW offsets still return to the same
  visible caret location.
- Gutter/minimap line layout still follows the live editor surface geometry via `positionToRectangle(...)`, but the
  logical line content now comes from the visible plain-text projection rather than the literal RAW tag string.
- That logical-line geometry now also routes through `ContentsLogicalLineLayoutSupport.js`, which maps every sampled
  line rectangle back into the block's own coordinate space before the structured flow caches it.
  Gutter Y therefore no longer depends on the inline editor's internal local origin accidentally matching the block
  host origin.
- The block still emits only RAW mutation requests upward; the rendered overlay remains a read-side projection and
  never becomes the persistence authority.
- The block now also exposes `clearSelection(preserveFocusedEditor)` so structured-flow-wide selection cleanup can
  drop stale paragraph highlight while leaving the actively focused editor alone.
- The nested inline editor now also runs one host-owned shortcut handler before its local boundary-navigation logic.
  Note-wide shortcuts such as clipboard-image paste can therefore be intercepted while focus is inside a structured
  paragraph editor, without reintroducing legacy whole-note editing authority.
- The block now also receives `paperPaletteEnabled` from the structured-flow host.
  In page/print mode the inline editor base text color is forced to paper black and the inline-style HTML overlay is
  re-rendered through the renderer's paper palette, so semantic/title/highlight spans cannot stay white on a white
  paper surface.
- An empty text block now treats plain `Backspace` / `Delete` as "remove this line" before it tries adjacent atomic
  block deletion.
  Zero-length paragraph blocks therefore no longer become undeletable cursor anchors.
- Plain `Enter` inside `paragraph` / `p` blocks is no longer intercepted by the shared inline editor wrapper.
  The live `TextEdit` receives the key directly, then the ordinary text-edit mutation path persists the resulting RAW
  source snapshot.
- Flattened interactive prose spans intentionally opt out of that paragraph-boundary interception at the host layer.
  For those grouped spans, native newline insertion stays inside the shared prose editor and the parser can rediscover
  paragraph boundaries on the next RAW refresh pass.
- The old paragraph-boundary Backspace/Delete helper remains in the delegate for non-native block hosts, but
  `ContentsInlineFormatEditor.qml` no longer dispatches native text-input keys through that helper.
  Repeated Backspace/Delete therefore stays with the OS/Qt `TextEdit` path while the user is editing text.

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
- Paragraph split/merge requests still never mutate RAW locally inside this delegate.
  They only emit the boundary intent upward so the shared structured mutation policy can rewrite implicit lines and
  explicit paragraph wrappers consistently from one place.
- Inline-format command handling now follows the same rule:
  this delegate exposes selection/focus information, but the structured editor controller owns the RAW formatting
  mutation.
