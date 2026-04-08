# `src/app/qml/view/panels/navigation/view/NavigationApplicationViewModeBar.qml`

## Responsibility
`NavigationApplicationViewModeBar.qml` owns the Figma `258:7849` `ModeBar` slice for the view-mode
application toolbar.

## Figma Mapping
- Frame: `258:7849` `ModeBar`
- Button order:
  - `258:7852` `CenterView` -> `singleRecordView`
  - `258:7853` `FocusMode` -> `imagefitContent`
  - `258:7854` `Presentation` -> `runshowCurrentFrame`

## Interaction Contract
- Root type: `LV.HStack`
- Exposes `viewHookRequested(string reason)` and `requestViewHook(reason)`.
- All three actions are plain `LV.IconButton` controls with borderless tone and `gap2` padding so the
  metadata-level icon contract stays explicit in QML.

## Regression Checklist
- Keep the mode button order `CenterView -> FocusMode -> Presentation`.
- Do not swap the `FocusMode` icon away from `imagefitContent`.
