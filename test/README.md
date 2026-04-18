# Regression Tests

This directory owns the in-repo automated regression suite for WhatSon.

The maintained regression surface combines root-level build gates with a Qt Test based C++ runtime suite under
`test/cpp/`.

The `test/CMakeLists.txt` entrypoint now stays intentionally thin and delegates the executable/test wiring to
`test/cpp/CMakeLists.txt`, so additional regression surfaces can be added without growing one monolithic test build
definition.

- `whatson_build_regression` builds the maintained product binaries and the regression test executable in `build/`.
- `whatson_cpp_regression` runs the runtime C++ regression assertions only.
- `whatson_regression` is the default combined verification gate.

The C++ suite currently locks regression-sensitive runtime behavior for:

- `SelectedHubStore`
- `SidebarSelectionStore`
- `HierarchyViewModelProvider`
- `SidebarHierarchyViewModel`
- `NavigationModeViewModel`
- `EditorViewModeViewModel`
- `OnboardingRouteBootstrapController`
- `WhatSonCronExpression`
- `WhatSonAsyncScheduler`
- `WhatSonUnixTimeAnalyzer`
- `ContentsResourceTagTextGenerator`
- `WhatSonNoteFolderSemantics`
- `ContentsStructuredDocumentCollectionPolicy`
- `ContentsStructuredDocumentHost`
- `ContentsStructuredDocumentMutationPolicy`
- `ContentsLogicalLineLayoutSupport.js`
- `WhatSonClipboardResourceImportFileNamePolicy`
- `ContentsEditorSessionController`
- `ContentsNoteManagementCoordinator`

The suite avoids booting the full application shell or loading a hub package.

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

- The QML-side line-coordinate helper `ContentsLogicalLineLayoutSupport.js` is now regression-tested through
  `QJSEngine`, so gutter/minimap line placement keeps the block-local mapped Y contract even though the maintained
  runtime suite stays C++-driven.
- Structured editor selection cleanup is now also locked at the C++ host-object layer, so block activation keeps
  emitting the selection-clear revision/retained-block contract that the QML delegates consume.
- Clipboard-image resource imports now also pin their synthesized asset-name policy in the C++ suite, so temporary
  placeholder names cannot regress back to a collision-prone fixed file name.
