# `src/app/file/hub/WhatSonHubPackager.hpp`

## Role
Declares the package-root service for `.wshub` creation.

## Public API
- `packageExtension()`: returns the canonical `.wshub` suffix.
- `normalizePackagePath(...)`: converts a requested hub path into an absolute `.wshub` package path.
- `createPackageRoot(...)`: creates the package directory and applies any platform-specific package presentation rules.

## Packaging Contract
- Package-root creation is separate from hub scaffold creation.
- Existing package directories are treated as hard failures and are never overwritten in place.
- Apple-specific package presentation is delegated behind this boundary instead of being embedded in `WhatSonHubCreator`.
