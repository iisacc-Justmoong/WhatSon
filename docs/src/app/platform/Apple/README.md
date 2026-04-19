# `src/app/platform/Apple`

## Status
- Directory mirror generated from the current `src` tree.
- This file is the entry point for the detailed documentation pass of this directory.

## Scope
- Mirrored source directory: `src/app/platform/Apple`
- Child directories: 0
- Child files: 7

## Child Directories
- No child directories.

## Child Files
- `AppleSecurityScopedResourceAccess.hpp`
- `AppleSecurityScopedResourceAccess.mm`
- `WhatSonIosHubPickerBridge.cpp`
- `WhatSonIosHubPickerBridge.hpp`
- `WhatSonIosHubPickerBridge.mm`
- `WhatSonApplePackageAppearance.hpp`
- `WhatSonApplePackageAppearance.mm`

## Intended Detailed Sections
- Module responsibilities and architectural layer
- Internal submodule boundaries
- Cross-directory dependencies
- Runtime ownership and lifecycle rules
- Testing strategy and coverage map
- Known hotspots and refactor priorities

## Notes
- `WhatSonApplePackageAppearance` owns the Apple-native `NSURLIsPackageKey` presentation hint used when a newly created
  `.wshub` directory should appear as a file-like package in Finder and other Apple document browsers.
