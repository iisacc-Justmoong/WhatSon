# iOS

This directory contains iOS platform-specific bundle configuration.

- `Info.plist` must not declare the legacy `CFBundleIconFile` key. The app icon is provided by the Xcode asset
  catalog generated from `resources/icons/app/ios/*` via `src/app/CMakeLists.txt`, and that `.xcassets` directory must
  be attached to the bundle `Resources` phase so Xcode emits `Assets.car`.
- `Info.plist` keeps `UIRequiresFullScreen` enabled because the current iOS target only declares portrait /
  landscape-left / landscape-right orientations.
- When no startup `.wshub` is available on mobile, app bootstrap uses the standalone onboarding window path before
  loading `Main.qml`.
- `Main.qml` must not import macOS-only `Qt.labs.platform` directly. The native menu bar lives in
  `src/app/qml/window/MacNativeMenuBar.qml` so iOS static QML linking does not pull that module into the root shell.
