# `src/app/platform/Apple/WhatSonApplePackageAppearance.mm`

## Role
Implements the Apple-native package presentation pass for `.wshub` directories.

## Behavior
- Validates that the incoming path is a real directory.
- Builds an `NSURL` for the directory and sets `NSURLIsPackageKey` to `YES`.
- Returns a descriptive error when Foundation cannot apply the package hint.

## Integration
- Called only through `WhatSonHubPackager`.
- Keeps Apple-specific package behavior out of `WhatSonHubCreator`, which now focuses on scaffold writes.
