# `src/app/qml/view/panels/detail/DetailContents.qml`

## Responsibility
`DetailContents.qml` owns the state-switched body of the desktop detail panel.
The implemented primary forms are the Figma `Properties` example and the live `fileStat` statistics form.

## Root Contract
- Root `objectName`: `DetailContents`
- Inputs include the active content controller, detail panel controller, file stat controller, and hierarchy selector controllers.
- Canonical states are `detached`, `properties`, `fileStat`, `insert`, `fileHistory`, `layer`, and `help`.
- Legacy aliases are not accepted.

## Implemented Properties Form
- `Projects`, `Bookmark`, and `Progress` selectors read from dedicated detail-panel-local selector controllers.
- `FoldersList` and `TagsList` mirror persisted `.wsnhead` metadata and write through `DetailPanelController`.
- The shared `DetailMetadataHierarchyPicker.qml` renders assignable hierarchy entries through an LVRS context menu.

## LVRS Reuse
- Uses `LV.ComboBox`, `LV.ContextMenu`, `LV.HierarchyItem`, and `LV.ListFooter`.
- Uses LVRS typography and panel tokens instead of ad-hoc colors or fonts.
- Compact rows, list cards, footer bars, and form insets derive their size from LVRS theme tokens.

## File Statistics State
- The `fileStat` state mounts `DetailFileStatForm.qml`.
- The statistics surface renders persisted `.wsnhead <fileStat>` values and current header metadata.

## Detached State
- `detached` closes metadata pickers and inline folder editing, then renders no form content.
