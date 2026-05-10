# Regression Tests

This directory owns the in-repo automated regression suite for WhatSon.

The maintained regression surface combines root-level build gates with a Qt Test based C++ runtime suite under
`test/cpp/`.

## Entrypoints

- `whatson_build_regression` builds the maintained product binaries and the regression test executable in `build/`.
- `whatson_cpp_regression` runs the runtime C++ regression assertions only.
- `whatson_regression` is the default combined verification gate.

## Commands

Configure the repository in the standard build tree:

```bash
cmake -S . -B build
```

Run the maintained build gate:

```bash
cmake --build build --target whatson_build_regression -j
```

Run the default combined regression target:

```bash
cmake --build build --target whatson_regression -j
```

Run the same suite through `ctest`:

```bash
ctest --test-dir build --output-on-failure -L cpp_regression
```

## Current Focus

- Runtime editor QML coverage pins the center editor route to exactly three contents views: `Gutter.qml`,
  `TextEditor.qml`, and `Minimap.qml`; `TextEditor.qml` is rooted in `LV.TextEditor` with an empty `filePath`.
- Source-tree policy coverage requires the explicit `src/app/models/editor` CMake shard while still rejecting removed
  editor minimap backends and any extra contents-view QML beyond those three files.
- The same coverage now verifies that `Main.qml` keeps the restored desktop/mobile shell while the removed editor
  view-mode selector/controller family stays absent.
- The remaining C++ suite covers app launch, LVRS context binding, hierarchy/navigation controllers, note-list
  selection state, resource/package utilities, note file persistence, startup hub resolution, scheduler support, and
  mobile routing coordinator units.
- The suite avoids booting the full application shell or loading a hub package.
