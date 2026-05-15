# `src/app/models/hierarchy/projects/ProjectsHierarchyModel.hpp`

## Responsibility

This header keeps the typed `ProjectsHierarchyItem` struct and icon helper used by `ProjectsHierarchyController`
internals.

## Shared Model Contract

- It no longer declares a Qt item model class.
- The QML/LVRS-facing model is the shared `WhatSonHierarchyModel`.
- Nested project depth, expansion, and drag flags are serialized by the controller into `LV.Hierarchy` node maps.
