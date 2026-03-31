# `src/app/qml/view/calendar/CalendarTodayControl.qml`

## Role
`CalendarTodayControl.qml` provides the shared `Prev / Today / Next` navigation control used by all calendar pages
(`Day`, `Week`, `Month`, `Year`).

## Figma Alignment
- Reference node: `227:8807`
- Layout contract:
  - horizontal stack spacing: `LV.Theme.gap2`
  - previous/next icon buttons: `horizontalPadding = LV.Theme.gap2`, `verticalPadding = LV.Theme.gap2`
  - today button: `horizontalPadding = LV.Theme.gap8`, `verticalPadding = LV.Theme.gap4`, `font.pixelSize = 12`
  - button tone uses `LV.AbstractButton.Default`

## Public QML Contract
- `property string todayText`: label for center action button (`"Today"` default)
- `signal previousRequested`
- `signal todayRequested`
- `signal nextRequested`
- `signal viewHookRequested(string reason)`

## Interaction
1. Previous icon emits `previousRequested` and `viewHookRequested("previous")`.
2. Today label button emits `todayRequested` and `viewHookRequested("today")`.
3. Next icon emits `nextRequested` and `viewHookRequested("next")`.

## Collaborators
- `src/app/qml/view/calendar/DayCalendarPage.qml`
- `src/app/qml/view/calendar/WeekCalendarPage.qml`
- `src/app/qml/view/calendar/MonthCalendarPage.qml`
- `src/app/qml/view/calendar/YearCalendarPage.qml`
