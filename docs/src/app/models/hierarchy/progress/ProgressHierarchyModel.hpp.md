# `src/app/models/hierarchy/progress/ProgressHierarchyModel.hpp`

## Responsibility

This header keeps the typed `ProgressHierarchyItem` struct and icon helper used by `ProgressHierarchyController`
internals.

## Shared Model Contract

- It no longer declares a Qt item model class.
- The QML/LVRS-facing model is the shared `WhatSonHierarchyModel`.
- Progress state labels and values are serialized by the controller into `LV.Hierarchy` node maps.
