# `src/app/platform/Apple/WhatSonApplePackageAppearance.hpp`

## Role
Declares the Apple-native helper that marks a `.wshub` directory as a package.

## Public API
- `applyPackageDirectoryPresentation(...)`: applies the Apple package presentation hint to an already created
  directory path.

## Boundary
- This helper is intentionally isolated under `platform/Apple` so `WhatSonHubPackager` can stay in portable C++ while
  still delegating the Finder/document-browser package behavior to Objective-C++.
