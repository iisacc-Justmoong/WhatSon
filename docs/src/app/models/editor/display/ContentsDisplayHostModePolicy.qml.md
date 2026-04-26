# `src/app/models/editor/display/ContentsDisplayHostModePolicy.qml`

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

## Collaborators

- `ContentsDisplayView.qml`: mounts this policy and binds its mode-specific tokens/flags.
- `ContentViewLayout.qml`: forwards `isMobilePlatform` into `ContentsDisplayView.mobileHost`.
- `ContentsDisplaySurfacePolicy`: owns document-surface selection; this file only owns desktop/mobile chrome deltas.

## Regression Checks

- Switching `mobileHost` must not require a second editor-host file.
- Mobile mode must not reserve gutter width or run desktop structured minimap/gutter refresh loops.
- Desktop mode must keep gutter/minimap behavior unchanged.
