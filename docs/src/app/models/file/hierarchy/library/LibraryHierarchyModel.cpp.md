# `src/app/models/file/hierarchy/library/LibraryHierarchyModel.cpp`

## Responsibility

This implementation projects `LibraryHierarchyItem` rows into a Qt model consumable by LVRS/QML.

## UUID-Specific Behavior

- `ItemKeyRole` returns `folder:<uuid>` for persisted user folders when a UUID is available.
- reserved smart buckets still use their historical stable keys.
- model sanitization clears `folderUuid` from system buckets because those items are not backed by
  persisted folder rows.

## Why This Matters

The visible tree can be reordered, renamed, and rebound by QML delegates. Stable item keys prevent
selection and rename overlays from drifting when the display path changes.
