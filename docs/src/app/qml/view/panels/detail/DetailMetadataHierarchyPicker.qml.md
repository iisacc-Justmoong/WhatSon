# `src/app/qml/view/panels/detail/DetailMetadataHierarchyPicker.qml`

## Responsibility
`DetailMetadataHierarchyPicker.qml` owns the shared folder/tag add overlay used by `DetailContents.qml`.
It subclasses `LV.ContextMenu`, keeps the LVRS context-menu container contract intact, and overrides only the menu body
with a fully expanded hierarchy-item list. The container geometry still adapts to the active form factor:
- desktop: anchored context-menu-style popup
- mobile: bottom-sheet-style popup

## Inputs
- `emptyStateText`
- `manualFallbackText`
- `hierarchyItems`
- `manualFallbackEnabled`

## Behavior
- `openForAnchor(anchorItem, reason)` opens the overridden `LV.ContextMenu` and reports the hook reason back to the parent.
- The component does not wrap a standalone `Controls.Popup`; instead it reuses LVRS context-menu animation, dismissal,
  overlay parenting, padding, and width contracts while replacing only the body renderer and mobile sheet placement.
- Hierarchy rows are rendered as `LV.HierarchyItem` delegates directly inside the menu body, so the popup keeps a
  context-menu shell instead of embedding another hierarchy panel surface.
- The menu body uses `Controls.ScrollBar`, because this file imports `QtQuick.Controls` with the `Controls` alias and
  must keep that type resolution explicit to remain loadable inside the QML module.
- Every supplied hierarchy item is treated as already expanded and non-collapsible: the picker never shows chevrons and
  never mutates source hierarchy expansion state.
- Clicking a hierarchy row resolves the filtered picker entry and emits `entryChosen(entry)` instead of mutating note
  metadata locally.
- When `manualFallbackEnabled == true`, the footer row emits `manualFallbackRequested()` and lets the parent reopen the
  inline folder editor.
- Popup margins, row height, and fallback width now use named `LV.Theme` gap/control-width tokens.

## Tests

- Automated test files are not currently present in this repository.
- Regression checklist:
  - Desktop popup geometry must clamp inside the overlay even when the footer add button sits near the right or bottom edge.
  - Mobile sheet geometry must stay inset from the screen edges while still opening through the overridden `LV.ContextMenu`.
  - The picker must render every hierarchy node in the source order immediately, without chevrons or collapse affordances.
  - Clicking a rendered hierarchy row must emit `entryChosen(...)` and close the menu.
  - Closing the popup must clear the temporary anchor state before the next open.
