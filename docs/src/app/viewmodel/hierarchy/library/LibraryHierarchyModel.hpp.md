# `src/app/viewmodel/hierarchy/library/LibraryHierarchyModel.hpp`

## Responsibility

This header defines the Qt item model that backs the visible library folder tree in QML.

## UUID In The UI Model

`LibraryHierarchyItem` now carries a `folderUuid` field in addition to its visible label, icon, and
legacy path information. That lets the model expose a stable item key for normal folders even when a
rename changes the rendered text.

## Key Contract

- system buckets such as All, Draft, and Today keep their reserved keys
- regular folders prefer `folder:<uuid>` as the item key

This key contract is important because QML selection, LVRS reorder flows, and viewmodel lookup code
need an identity that survives path edits.
