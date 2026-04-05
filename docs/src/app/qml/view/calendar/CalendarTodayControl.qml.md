# `src/app/qml/view/calendar/CalendarTodayControl.qml`

## Role
`CalendarTodayControl.qml` provides the shared `Prev / Today / Next` navigation control used by all calendar pages
(`Day`, `Week`, `Month`, `Year`).

## Figma Alignment
- Reference node: `238:7843`
- Layout contract:
  - horizontal stack spacing: `LV.Theme.gap2`
  - all three buttons use `LV.Theme.scaleMetric(20)` for the button extent and `LV.Theme.scaleMetric(16)` for the icon extent
  - button padding stays `2` on both axes via `LV.Theme.gap2`
  - button background is `LV.Theme.panelBackground12` across idle/hover/pressed/disabled states
  - previous/next use `generalchevronUpLarge` rotated `-90` / `90`
  - center action uses the LVRS `threadAtBreakpoint` icon to match the Figma orange-circle glyph
  - button corner radius stays `LV.Theme.radiusSm`
  - buttons use `LV.AbstractButton.Borderless` with explicit background colors

## Public QML Contract
- `signal previousRequested`
- `signal todayRequested`
- `signal nextRequested`
- `signal viewHookRequested(string reason)`

## Interaction
1. Previous icon emits `previousRequested` and `viewHookRequested("previous")`.
2. Center icon button emits `todayRequested` and `viewHookRequested("today")`.
3. Next icon emits `nextRequested` and `viewHookRequested("next")`.

## Tests
- Automated test files are not currently present in this repository.
- Regression checklist:
    - The shared calendar control must render as three LVRS-scaled icon buttons without a text `Today` label.
    - The center button must use the orange `threadAtBreakpoint` glyph.
    - Previous and next buttons must keep the rotated chevron treatment from the Figma node.

## Collaborators
- `src/app/qml/view/calendar/DayCalendarPage.qml`
- `src/app/qml/view/calendar/WeekCalendarPage.qml`
- `src/app/qml/view/calendar/MonthCalendarPage.qml`
- `src/app/qml/view/calendar/YearCalendarPage.qml`
