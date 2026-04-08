# `src/app/qml/view/panels/navigation/view/NavigationApplicationViewCalendarBar.qml`

## Responsibility
`NavigationApplicationViewCalendarBar.qml` owns the view-only calendar cluster for Figma `149:4001`
inside `ApplicationViewBar`.

## Figma Mapping
- Frame: `149:4001` `CalendarBar`
- Button order:
  - `149:4002` `TodoListButton` -> `validator`
  - `149:4003` `DailyCalButton` -> `newUIlightThemeSelected`
  - `149:4004` `WeeklyCalButton` -> `table`
  - `149:4005` `MonyhltCalButton` -> `pnpm`
  - `149:4006` `YearlyCalButton` -> `runshowCurrentFrame`

## Interaction Contract
- Root type: `LV.HStack`
- Exposes `viewHookRequested(string reason)` and `requestViewHook(reason)`.
- The first button keeps the existing agenda hook reason (`view-open-agenda`) while switching the icon/name contract
  to the newer Figma `TodoListButton -> validator` metadata.

## Regression Checklist
- Keep the `validator` icon on `todoListButton`; do not fall back to `toolWindowCheckDetails`.
- Preserve the five-button order so compact-menu parity remains predictable.
