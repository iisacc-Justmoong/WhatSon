# `src/app/file/hub/WhatSonHubPackager.cpp`

## Role
Implements `.wshub` package-root materialization independently from hub scaffold creation.

## Responsibilities
- Normalizes requested hub paths so package creation always lands on an absolute `.wshub` directory.
- Creates the parent directory and the package root directory.
- Applies the Apple package presentation hint through `WhatSonApplePackageAppearance` when the build runs on Apple platforms.

## Behavior Notes
- Package creation rejects non-local targets because `.wshub` creation writes a real directory tree.
- Apple package presentation failures are logged as warnings so onboarding can still finish package creation when a
  provider filesystem does not persist Finder-style package metadata.

## Tests
- `test/cpp/whatson_cpp_regression_tests.cpp`
  includes both runtime coverage for package-root creation and source-level regression coverage for the creator/packager split.
