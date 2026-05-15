# `src/app/models/hierarchy/bookmarks/BookmarksHierarchyModel.hpp`

## Responsibility

This header keeps the typed `BookmarksHierarchyItem` struct and icon helper used by `BookmarksHierarchyController`
internals.

## Shared Model Contract

- It no longer declares a Qt item model class.
- The QML/LVRS-facing model is the shared `WhatSonHierarchyModel`.
- Bookmark color/icon metadata remains on the typed struct and is published through controller `depthItems()` maps.
