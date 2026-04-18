# `CMakeLists.txt`

## Responsibility
- Owns repository-wide toolchain setup, product build options, and the top-level `add_subdirectory(...)` graph.
- Keeps application, daemon, CLI, and `test/` enablement close to the root configuration entrypoint.
- Delegates large custom-target groups to `cmake/root/*/CMakeLists.txt` so the root file remains an orchestration surface instead of a full target catalog.

## Current Root Split
- `cmake/root/build/CMakeLists.txt`: maintained regression targets such as `whatson_build_regression` and `whatson_regression`.
- `cmake/root/dev/CMakeLists.txt`: developer tooling targets such as `whatson_qmllint`, `whatson_qmlformat_*`, and `whatson_clang_tidy`.
- `cmake/root/runtime/CMakeLists.txt`: run, healthcheck, mobile launch/export aliases, and iOS Xcode-project generation.
- `cmake/root/distribution/CMakeLists.txt`: install/export/package targets and the dedicated `build-trial` mirror flow.

## Invariants
- Keep option declarations, package discovery, and primary product `add_subdirectory(...)` calls in the root file.
- Keep grouped custom targets in `cmake/root/*` and avoid moving product-level source ownership back into the root file.
- Reuse `build/` for configure/build/test flows; the nested `build-trial` path remains opt-in packaging infrastructure only.

## Verification Notes
- Run `cmake -S . -B build` after structural root-CMake changes.
- Run `cmake --build build --target whatson_build_regression -j` after root-target refactors.
- Run `cmake --build build --target whatson_regression -j` when regression-target wiring changes.
