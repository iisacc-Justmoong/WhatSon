# `src/app/qml/view/panels/navigation/NavigationEditorViewBar.qml`

## Responsibility
Renders the restored navigation bar view-mode combo box.

## Surface
- Uses `LV.ComboBox` and `LV.ContextMenu`.
- Shows `Plain`, `Page`, `Print`, `Web`, and `Presentation` from `editorViewModeController`.
- Emits the standard view hook when the menu opens or a mode is selected.

## Boundary
This component is navigation chrome. It updates `EditorViewModeController` selection state but does not mount editor
renderers or change `ContentViewLayout.qml`.
