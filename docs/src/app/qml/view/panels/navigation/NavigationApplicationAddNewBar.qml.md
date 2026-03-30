# `src/app/qml/view/panels/navigation/NavigationApplicationAddNewBar.qml`

## Responsibility
`NavigationApplicationAddNewBar.qml` is the shared application-level add-new segment wrapper.

Both `NavigationApplicationViewBar.qml` and `NavigationApplicationEditBar.qml` use this single
wrapper so add-new behavior is shared without per-mode duplicate wrappers.

## Composition
- Root: `NavigationAddNewBar`
- Source dependency: `src/app/qml/view/panels/navigation/NavigationAddNewBar.qml`
