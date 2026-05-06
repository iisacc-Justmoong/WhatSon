# `src/app/models/editor/input/ContentsWysiwygEditorPolicy.cpp`

## Responsibility

Implements WYSIWYG editor policy for the inline note body surface.

## Behavior

- Parses source tag tokens in C++ to decide whether a selected RAW range represents visible logical content.
- Uses the supplied C++ coordinate mapper to translate visible logical selections back to RAW source offsets.
- Trims mapped RAW selections past hidden opening inline wrappers and before hidden closing wrappers, so a range that
  contains only formatting tags stays a collapsed caret on the visible editor surface.
- Builds visible-text mutation payloads from the native logical text delta. Inserted visible text is escaped for RAW
  `.wsnbody`, deleted/replaced ranges skip hidden inline wrapper boundaries, and committed web-link text can still be
  promoted through the canonical note-body web-link support.
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
