# `src/app/models/hierarchy/library/LibraryHierarchyModel.hpp`

## Responsibility

This header keeps the typed `LibraryHierarchyItem` struct and `libraryHierarchyIconName(...)` helper used by
`LibraryHierarchyController` internals.

## Shared Model Contract

- It no longer declares a Qt item model class.
- The QML/LVRS-facing model is the shared `WhatSonHierarchyModel`.
- Library-specific row identity remains in the struct: system buckets keep reserved identities and normal folders carry
  `folderUuid`/`folderPath` so controller serialization can produce stable `LV.Hierarchy` node keys.
