# `src/app/qml/view/panels/ContentEditorToolbar.qml`

## Role
`ContentEditorToolbar.qml` is the LVRS-only editor toolbar mounted at the top of `ContentViewLayout.qml`.
It mirrors the Figma `EditorToolbar` frame (`399:9846`) as a compact 784x20 content row with 2px padding on every side
while staying a view-only command surface.

## Figma Contract
- Root frame: `EditorToolbar`, node `399:9846`, `784 x 20`, horizontal, space-between. The QML root adds
  `LV.Theme.gap2` padding around that Figma content frame.
- Left group: `StyleBar`, node `398:8628`, `664 x 20`, 12px spacing.
- Selector instances: `style` (`397:8570`), `font` (`399:8668`), `fontSize` (`399:8663`), `fontWeight`
  (`399:8673`), and `lineHeight` (`399:8678`) are LVRS `ComboBox` controls with `Arrow=Down` and `Tone=primary`.
- Format group: `formatBar`, node `398:8627`, uses LVRS icon buttons for `bold`, `italic`, `underline`,
  `strikethrough`, and `highlight`.
- Color group: `colorBar`, node `399:9827`, uses LVRS menu buttons for text color and background swatches.
- Right group: `Frame 1000000891`, node `400:8662`, contains `generaladd` (`399:9835`) and active `rendererKit`
  (`400:8656`) toggle buttons.
- Token values are preserved through LVRS theme tokens where they already exist: `panelBackground10`, `panelBackground12`,
  `primary`, body typography, `gap20`, `gap12`, and `gap4`. Figma-only swatch colors are kept locally as read-only
  metadata values.

## Behavior
The toolbar exposes only view-level signals:
- `formatTagRequested(tagName)` for the existing editor format dispatch in `ContentViewLayout.qml`.
- `styleTagStyleRequested(styleValue)` when the left `style` selector chooses a `<style ...>` tag `style`
  attribute value.
- `toolbarActionRequested(actionName)` for view-local hooks such as selector/menu button clicks.

The leftmost `style` selector opens an LVRS `ContextMenu` instead of behaving as a bare button. Its string values are
`Title`, `Title2`, `Subtitle`, `Header`, `Header2`, `Body`, `Description`, `Caption`, and `Footnote`. `Body` is the
display fallback for an empty `style` attribute. Selecting an item updates only the selector display state and emits the
selected string; `ContentViewLayout.qml` forwards that value to the C++ session API so source mutation remains outside
this toolbar surface.
The menu items use a custom LVRS `MenuItem` delegate that previews each style with the same LVRS typography token
descriptor used by the style component contract. For example, `Title` binds to `LV.Theme.textTitle`, so the context
menu row displays the `Title` label at that token's pixel size instead of using the default menu text size. The same
descriptor also supplies token-aligned weight, style name, line height, and letter spacing for the preview row. Style
preview descriptors may mix size and weight tokens instead of staying inside one text token; for example `Title`
previews as `textTitle` plus `textTitleWeight`, while `Header` previews as a bold `textHeader` variant. The context menu
preview intentionally carries no per-style color metadata, leaving item text color to the default LVRS label/menu
state. Style selector menu entries disable the LVRS `MenuItem` left icon slot through `showIconSlot: false`, so the
preview text starts without the default menu icon placeholder.

When the available width drops below the full Figma content width, the left-side controls are hidden as discrete
on/off items from left to right instead of clipping the right edge. The collapse order is `style`, `font`, `fontSize`,
`fontWeight`, `formatBar`, `colorBar`, then `lineHeight`; the right mode group remains the anchored minimum surface.

It does not read files, own note/session state, parse editor text, or persist formatting. All source mutation remains in
`NoteEditorDocumentSession`, `SetTag`, and the note-body persistence layer.

## Guardrails
- Keep root and layout primitives in LVRS (`LV.HStack`, `LV.ComboBox`, `LV.IconButton`, `LV.IconMenuButton`).
- Keep the style selector popup on LVRS `ContextMenu`/`MenuItem` delegates and bind preview size, weight, style name,
  and line height to LVRS theme tokens instead of hard-coded QML visual values. Do not add per-style colors to the
  context menu item previews.
- Keep the style selector popup's `showIconSlot: false` binding in place; do not fake this by painting over the icon
  area or shrinking unrelated menu metrics.
- Preserve Figma node ids as QML metadata properties when adding toolbar controls.
- Do not introduce `QtQuick.Controls`, `TextEdit`, parser/projection logic, or note session wiring here.

## Tests
- `test/cpp/suites/qml_contents_view_tests.cpp`
