# `CMakeLists.txt`

## Responsibility
- Owns repository-wide toolchain setup, product build options, and the top-level `add_subdirectory(...)` graph.
- Declares the desktop `WhatSon` Qt executable target when `WHATSON_BUILD_APP` is enabled.
- Finalizes the app QML module after `src/app` has collected QML entries.
- Delegates grouped target definitions to `cmake/root/*/CMakeLists.txt` so the root file remains an orchestration surface.
- Owns root-scope `POST_BUILD` hooks that must live in the same directory as the root-created app target.

## Current Root Split
- `cmake/root/build/CMakeLists.txt`: maintained regression gates, CTest integration, and `whatson_clean_build_extras`.
- `cmake/root/dev/CMakeLists.txt`: developer tools such as QML lint, QML format, and clang-tidy targets.
- `cmake/root/runtime/CMakeLists.txt`: desktop run and healthcheck targets.
- `cmake/root/distribution/CMakeLists.txt`: install, export, package, and trial mirror targets.

## Invariants
- Keep option declarations, package discovery, root target declaration, QML finalization, and primary product
  `add_subdirectory(...)` calls in this root file.
- Keep app source/module ownership below `src/app`.
- Reuse `build/` for configure, build, and test flows.
- Keep local `iiXml` and `iiHtmlBlock` package discovery host-side through cacheable root prefixes.
- Do not reintroduce separated platform export or launcher targets into this repository.

## Verification Notes
- Run `cmake -S . -B build` after structural root-CMake changes.
- Run `cmake --build build --target whatson_build_regression -j` after root-target refactors.
- Run `cmake --build build --target whatson_regression -j` when regression target wiring changes.
