# `src/app/models/hierarchy/preset/PresetHierarchyModel.hpp`

## Responsibility

This header keeps the typed `PresetHierarchyItem` struct used by `PresetHierarchyController` internals.

## Shared Model Contract

- It no longer declares a Qt item model class.
- The QML/LVRS-facing model is the shared `WhatSonHierarchyModel`.
- Preset names are transformed by the controller into named-string `LV.Hierarchy` node maps.
