# iOS

This directory contains iOS platform-specific bundle configuration.

- `Info.plist` must not declare the legacy `CFBundleIconFile` key. The app icon is provided by the Xcode asset
  catalog generated from `resources/icons/app/ios/*` via `src/app/CMakeLists.txt`.
- When no startup `.wshub` is available on mobile, app bootstrap uses the standalone onboarding window path before
  loading `Main.qml`.
