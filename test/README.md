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

- Runtime content QML coverage pins the center content route to `ImageEditor.qml` plus a blank placeholder for note and
  non-image selections.
- Source-tree policy coverage keeps the note editing, body persistence, raw sync, and editor view-mode object families
  absent from CMake, QML, docs, and the regression source list.
- Clipboard resource coverage verifies package import through `InAppClipboardManager` without editor paste wrappers or
  body tag insertion paths.
- The same coverage verifies that `Main.qml` keeps the desktop shell while removed editor and separated platform app
  object families stay absent.
- The remaining C++ suite covers app launch, LVRS context binding, hierarchy/navigation controllers, note-list
  selection state, resource/package utilities, note file persistence, startup hub resolution, and scheduler support.
- The suite avoids booting the full application shell or loading a hub package.
