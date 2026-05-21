# `CMakePresets.json`

## Responsibility
- Provides a repository-owned configure profile that CLion can import without relying on `.idea` state.
- Keeps the default local binary directory fixed at `build/`, matching the project build policy.
- Exposes maintained build gates through CMake build presets so IDE and shell workflows use the same target names.

## Presets
- `macos-clion`: configures the macOS development tree with the `Unix Makefiles` generator and `${sourceDir}/build`
  as `binaryDir`.
- `whatson-build-regression`: builds the maintained `whatson_build_regression` target from the `macos-clion`
  configure preset.
- `whatson-regression`: builds the maintained `whatson_regression` target from the `macos-clion` configure preset.

## Invariants
- Do not redirect the default preset away from `build/`.
- Do not commit `.idea` profile state to solve CLion configure issues; keep the reproducible contract in this preset
  file instead.
- Let the root `CMakeLists.txt` discover the local Qt installation through `QT_ROOT_PATH` and `~/Qt/6.*` instead of
  pinning a user-specific Qt patch version in this preset.

## Verification Notes
- Run `cmake --list-presets` after editing preset names or schema.
- Run `cmake --preset macos-clion` after changing configure cache variables.
- Run `cmake --build --preset whatson-build-regression` after build-preset changes.
