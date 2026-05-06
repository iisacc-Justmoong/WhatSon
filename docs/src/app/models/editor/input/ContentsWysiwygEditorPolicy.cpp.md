# `src/app/models/editor/input/ContentsWysiwygEditorPolicy.cpp`

## Responsibility

Implements WYSIWYG editor policy for the inline note body surface.

## Behavior

- Parses source tag tokens in C++ to decide whether a selected RAW range represents visible logical content.
- Uses the supplied C++ coordinate mapper to translate visible logical selections back to RAW source offsets.
- Builds rendered-overlay collapsed Backspace mutation payloads from visible logical cursor positions. The payload
  deletes the previous visible character range while skipping hidden opening inline tags at the range start and hidden
  closing inline tags at the range end, so Backspace cannot remove a `>` from `</bold>`/`</italic>` instead of the
  glyph under the projected cursor.
- Expands rendered selection ranges over iiHtmlBlock resource display blocks so a resource frame behaves as one
  atomic selection unit instead of exposing the RAW `<resource ... />` tag bytes.
- Produces cursor-normalization plans for hidden inline formatting tags. QML only applies the returned cursor position
  to the live `LV.TextEditor`.
- Provides visible line and paragraph range calculations used by rendered-surface double-click and triple-click
  selection.
