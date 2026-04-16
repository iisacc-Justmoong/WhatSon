# `src/app/qml/view/content/editor/ContentsDisplayHostModePolicy.qml`

## Responsibility

`ContentsDisplayHostModePolicy.qml` owns the desktop/mobile presentation deltas for the unified
`ContentsDisplayView.qml` host.

## Public Surface

- `mobileHost`
  Selects mobile-vs-desktop host mode.
- `editorHorizontalInset`
  Resolves body inset tokens per mode.
- `editorFontWeight`
  Resolves editor typography weight per mode.
- `showEditorGutter`
  Disables gutter chrome in mobile mode.
- `showMinimapRail`, `minimapRefreshEnabled`
  Gates minimap paint/update work per mode.
- `lineGeometryRefreshEnabled`
  Prevents the unified host from running desktop-only geometry refresh paths while mobile structured mode is active.
- `inlineEditorAutoFocusOnPress`
  Keeps native-input mobile mode scroll-first until the input session is actually active.
- `nativeInputDisplayText`
  Supplies the plain-text projection used by the shared inline editor in native-input mode.

## Collaborators

- `ContentsDisplayView.qml`: mounts this policy and binds its mode-specific tokens/flags.
- `ContentViewLayout.qml`: forwards `isMobilePlatform` into `ContentsDisplayView.mobileHost`.
- `ContentsInlineFormatEditor.qml`: consumes typography, autofocus, and plain-text input policy through the host.

## Regression Checks

- Switching `mobileHost` must not require a second editor-host file.
- Mobile mode must not reserve gutter width or run desktop structured minimap/gutter refresh loops.
- Desktop mode must keep gutter/minimap behavior unchanged.
