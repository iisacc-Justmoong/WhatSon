# Chnagelog

## 2026-02-15

### Commits

- 3014baa2618cfd1c01ec10c86c68ad45d0a62442 (2026-02-15 20:31 +0900) Added initial project structure for WhatSon
- 1dda6b50310a895bcaa3989dffdb5799478be548 (2026-02-15 20:56 +0900) Introduce `build_all.py` script for multi-platform
  build automation
- 7c57ef73f459dd809cff3fa14af3ee00fbb4c164 (2026-02-15 22:19 +0900) Temp commit

### Summary Of Changes

- Established the initial repository structure with CMake build definitions, LVRS-based QML UI shells, app/daemon
  entrypoints, and platform scaffolding. (38 files, +2852)
- Added `scripts/build_all.py` for multi-platform builds, moved platform assets under `platform/`, and adjusted
  README/CMake/daemon code accordingly. (11 files, +944/-6)
- Expanded root CMake/README, added Apple `Info.plist` and entitlements, and refined build automation and app
  CMakeLists. (6 files, +405/-48)

### Notes

- Verification was not run for this log update.
