# `src/app/viewmodel/onboarding`

## Status
- Directory mirror generated from the current `src` tree.
- This file is the entry point for the detailed documentation pass of this directory.

## Scope
- Mirrored source directory: `src/app/viewmodel/onboarding`
- Child directories: 0
- Child files: 4

## Child Directories
- No child directories.

## Child Files
- `OnboardingHubController.cpp`
- `OnboardingHubController.hpp`
- `OnboardingRouteBootstrapController.cpp`
- `OnboardingRouteBootstrapController.hpp`

## Intended Detailed Sections
- Module responsibilities and architectural layer
- Internal submodule boundaries
- Cross-directory dependencies
- Runtime ownership and lifecycle rules
- Testing strategy and coverage map
- Known hotspots and refactor priorities

## Current Notes
- Automated C++ regression coverage now lives in `test/cpp/whatson_cpp_regression_tests.cpp`, locking embedded
  onboarding route commits, reopen/dismiss routing, workspace transition callbacks for
  `OnboardingRouteBootstrapController`, the unified main-window startup onboarding session in `main.cpp`, and the
  platform-specific picker paths used by `OnboardingContent.qml`.
