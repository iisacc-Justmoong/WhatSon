# Regression Tests

This directory owns the in-repo automated regression suite for WhatSon.

The maintained regression surface combines root-level build gates with a Qt Test based C++ runtime suite under
`test/cpp/`.

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
- `ContentsStructuredDocumentMutationPolicy`

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
