# `src/app/qml/view/panels/navigation/NavigationApplicationPreferenceBar.qml`

## Responsibility
`NavigationApplicationPreferenceBar.qml` is the shared application-level preference/detail-toggle
segment wrapper.

Both `NavigationApplicationViewBar.qml` and `NavigationApplicationEditBar.qml` use this single
wrapper so detail-toggle behavior is shared without per-mode duplicate wrappers.

## Composition
- Root: `NavigationPreferenceBar`
- Source dependency: `src/app/qml/view/panels/navigation/NavigationPreferenceBar.qml`
