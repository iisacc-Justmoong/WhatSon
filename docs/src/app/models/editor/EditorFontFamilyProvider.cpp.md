# `src/app/models/editor/EditorFontFamilyProvider.cpp`

## Behavior
The implementation uses `QFontDatabase::families()` as the single system font source, normalizes the family list for
menu display, and builds LVRS menu item descriptors with:

- `label`
- `fontFamily`
- `showIconSlot: false`
- `keyVisible: false`
- `eventName: "editor.toolbar.font"`
- `eventPayload.fontFamily`

The provider deliberately stops at menu data. Applying the selected family to the editor selection belongs to the next
editor source-mutation contract.
