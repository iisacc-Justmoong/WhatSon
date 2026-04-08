# `src/app/qml/view/panels/navigation/view/NavigationApplicationViewOptionBar.qml`

## Responsibility
`NavigationApplicationViewOptionBar.qml` owns the Figma `258:7963` `ViewOptionBar` slice inside the
view-mode `ApplicationViewBar`.

## Figma Mapping
- Frame: `258:7963` `ViewOptionBar`
- Button order:
  - `258:7964` `ReadOnlyToggle` -> `readerMode`
  - `258:8048` `WarpText` -> `textAutoGenerate`
  - `258:8039` `CenterView` -> `singleRecordView`
  - `258:7965` `TextToSpeech` -> `textToSpeech`
  - `258:7966` `PaperOption` -> `fileFormat`

## Interaction Contract
- Root type: `LV.HStack`
- Exposes `viewHookRequested(string reason)` and `requestViewHook(reason)`.
- `TextToSpeech` and `PaperOption` intentionally use `LV.IconMenuButton` because the Figma metadata includes the
  trailing dropdown affordance on both controls.

## Regression Checklist
- Keep the five buttons in the exact Figma metadata order.
- Keep the icon-name contract `readerMode -> textAutoGenerate -> singleRecordView -> textToSpeech -> fileFormat`.
- Preserve the `LV.Theme.gap2` spacing plus the menu-button `left=2 / right=4 / top=2 / bottom=2` padding contract.
