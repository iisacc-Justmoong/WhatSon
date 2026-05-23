# `src/app/models/editor/EditorFontFamilyProvider.hpp`

## Role
`EditorFontFamilyProvider` is the editor-domain C++ provider for system font families used by the toolbar font selector.

## Contract
- Exposes `fontFamilies` and `fontFamilyCount` as read-only QML properties.
- Exposes `fontFamilyMenuItems()` so QML can render an `LV.ContextMenu` without querying `QFontDatabase` itself.
- `refreshFontFamilies()` reloads Qt system font families, trims empty names, removes duplicates, sorts them for menu
  display, and emits `fontFamiliesChanged()` only when the visible list changes.
- `requestProviderHook(reason)` is a narrow hook signal for view-level menu telemetry; it does not mutate editor text.

## Boundaries
- This class reads system font metadata only.
- It does not inspect the live `LV.TextEditor` or parse note source. Applying the selected family to `.wsnbody` is
  owned by `NoteEditorDocumentSession.insertStyleFontTagIntoSource(...)`.
