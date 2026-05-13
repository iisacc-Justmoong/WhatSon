# `src/app/models/hierarchy/IHierarchyController.hpp`

## Role
This file defines the shared read-oriented QObject contract for hierarchy controllers.

It is intentionally limited to state that many consumers need:
- hierarchy nodes
- selected index
- item count
- load success state
- last load error
- access to the hierarchy item model and note-list model

## Public Surface
The `Q_PROPERTY` layer exists so QML can bind against a normalized hierarchy shape regardless of the backing domain.

The required virtual functions map to the lower-level implementation contract:
- `itemModel()`
- `noteListModel()`
- `selectedIndex()` and `setSelectedIndex(...)`
- `itemCount()`
- `loadSucceeded()`
- `lastLoadError()`
- `hierarchyModel()`
- `itemLabel(...)`

## Signal Bridging
`initializeHierarchyInterfaceSignalBridge()` is important. Concrete hierarchy controllers are expected to expose domain-local signals such as `selectedIndexChanged()` and `hierarchyModelChanged()`. This helper bridges those implementation signals back into the normalized interface-level signals consumed by generic QML.

## Shared Expansion Helpers
`setHierarchyItemExpanded(...)` and `setAllHierarchyItemsExpanded(...)` are protected parent helpers for concrete
hierarchy controllers. They centralize the right-chevron expansion rules that every hierarchy shares: reject invalid
indexes, reject rows without `showChevron`, no-op when the requested state is already current, and call the domain's
commit hook only after the in-memory item state has changed.

## What This Interface No Longer Does
This interface no longer exposes rename, create, delete, reorder, expansion, or note-drop operations as public
view-facing API. Those behaviors moved into dedicated capability interfaces so read-only consumers do not depend on
write-heavy APIs. The protected expansion helpers are implementation reuse for child controllers, not a public mutation
surface.

## Practical Effect
If a QML surface only needs to show hierarchy state, bind to this interface.
If it needs to mutate hierarchy state, combine this interface with one or more capability interfaces through a bridge.
