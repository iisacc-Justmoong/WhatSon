# `src/app/models/hierarchy/event/EventHierarchyModel.hpp`

## Responsibility

This header keeps the typed `EventHierarchyItem` struct used by `EventHierarchyController` internals.

## Shared Model Contract

- It no longer declares a Qt item model class.
- The QML/LVRS-facing model is the shared `WhatSonHierarchyModel`.
- Event names are transformed by the controller into named-string `LV.Hierarchy` node maps.
