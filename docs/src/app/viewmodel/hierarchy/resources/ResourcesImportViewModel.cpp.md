# `src/app/viewmodel/hierarchy/resources/ResourcesImportViewModel.cpp`

## Responsibility

The implementation completes three steps in order.

1. Filter the input URL list down to real local files.
2. Copy each file into a `.wsresource` package inside the current hub.
3. Rewrite `Resources.wsresources` and invoke the resources runtime reload callback.

## Import Semantics

- The source file name is preserved as the package asset file name.
- The package id is derived from the file base name and receives a suffix when it would collide with an existing
  package.
- `type`, `bucket`, and `format` are assigned by the rules in `WhatSonResourcePackageSupport.hpp`.

## Failure Rule

If package creation or `Resources.wsresources` rewriting fails, every package directory created during that turn is
rolled back. If persistence succeeds but the runtime refresh callback fails, the imported packages remain on disk and
the ViewModel emits a failure signal so the UI can report the problem to the user.

## Current Callers

- The macOS global menu bar `File > Import File...` action.
- The compatibility wrappers that keep older drag/drop-style callers on the same code path.
