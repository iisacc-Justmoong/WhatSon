# `src/app/qml/view/panels/navigation/view/NavigationApplicationViewCalendarBar.qml`

## Responsibility
`NavigationApplicationViewCalendarBar.qml` owns the view-only calendar cluster for Figma `149:4001`
inside `ApplicationViewBar`.

## Figma Mapping
- Frame: `149:4001` `CalendarBar`
- Button order:
  - `149:4002` `TaskButton` -> `validator`
  - `149:4003` `DailyCalButton` -> `newUIlightThemeSelected`
  - `149:4004` `WeeklyCalButton` -> `table`
  - `149:4005` `MonyhltCalButton` -> `pnpm`
  - `149:4006` `YearlyCalButton` -> `runshowCurrentFrame`

## Interaction Contract
- Root type: `LV.HStack`
- Exposes `viewHookRequested(string reason)` and `requestViewHook(reason)`.
- The restored leftmost task button emits `view-open-task`; it does not reintroduce the removed legacy hook.

## Regression Checklist
- Keep the `validator` icon on `taskButton`; do not fall back to `toolWindowCheckDetails`.
- Preserve the five-button order so compact-menu parity remains predictable.
