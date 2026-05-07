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
- Retains a visible logical Backspace payload helper for explicit policy tests and non-native command callers. Ordinary
  rendered-mode Backspace is not driven from the projected cursor; it stays on the native logical text edit path and is
  converted through the visible-text mutation payload after Qt commits the edit.
- Expands rendered selection ranges over iiHtmlBlock resource display blocks so a resource frame behaves as one
  atomic selection unit instead of exposing the RAW `<resource ... />` tag bytes.
- Produces cursor-normalization plans for hidden inline formatting tags. QML only applies the returned cursor position
  to the live `LV.TextEditor`.
- Produces cursor-normalization plans for atomic resource blocks. A collapsed caret that lands on the resource
  placeholder is moved to the nearest prose boundary outside the frame when such a boundary exists; resource-only
  frames remain non-cursorable and are reported as an active atomic-resource cursor zone so QML can hide the native
  caret instead of painting it inside the image frame.
- Provides visible line and paragraph range calculations used by rendered-surface double-click and triple-click
  selection.
