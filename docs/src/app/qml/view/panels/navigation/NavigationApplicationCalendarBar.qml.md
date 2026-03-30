# `src/app/qml/view/panels/navigation/NavigationApplicationCalendarBar.qml`

## Responsibility
`NavigationApplicationCalendarBar.qml` is the shared application-level calendar segment wrapper.

Both `NavigationApplicationViewBar.qml` and `NavigationApplicationEditBar.qml` use this single
wrapper so calendar controls are not duplicated per mode directory.

## Composition
- Root: `NavigationCalendarBar`
- Source dependency: `src/app/qml/view/panels/navigation/NavigationCalendarBar.qml`
