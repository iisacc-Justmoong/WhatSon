# `src/app/models/hierarchy/resources/ResourcesHierarchyModel.hpp`

## Responsibility

This header keeps the typed `ResourcesHierarchyItem` struct and `resourcesHierarchyIconName(...)` helper used by
`ResourcesHierarchyController` internals.

## Shared Model Contract

- It no longer declares a Qt item model class.
- The QML/LVRS-facing model is the shared `WhatSonHierarchyModel`.
- Resource taxonomy fields such as `kind`, `bucket`, `type`, `format`, and resource paths remain on the typed struct so
  the controller can serialize resource hierarchy nodes without adding a domain-specific view model.
