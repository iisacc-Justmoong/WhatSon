# `src/app/models/sidebar/SidebarHierarchyInteractionController.hpp`

## Role
`SidebarHierarchyInteractionController` exposes the C++ contract used by `SidebarHierarchyView.qml` for hierarchy
interaction policy that is not visual layout.
The type is registered as a creatable internal QML QObject, so the C++ class must remain non-`final`; Qt's QML
registration layer derives an internal `QQmlElement<T>` wrapper from it.

## Public Surface
- The class is intentionally not marked `final` because the internal QML registrar exposes it as a creatable QObject
  type and Qt wraps creatable types with `QQmlElement<T>`.
- `hierarchyInteractionBridge` binds the controller to the existing hierarchy CRUD/expansion bridge.
- `activeHierarchyIndex` scopes preserved expansion keys by active sidebar domain.
- `footerActionName(...)` and `requestFooterAction(...)` normalize footer button events and coalesce duplicate LVRS
  callback/signal dispatch from a single click.
- `captureExpansionState(...)`, `modelWithPreservedExpansion(...)`, `handleExpansionSignal(...)`, and
  `requestChevronExpansion(...)` keep row expansion state and mutation routing out of QML.
- `beginActivationAttempt(...)`, `activationAttemptCurrent(...)`, and `shouldSuppressActivation(...)` let the view defer
  activation safely after chevron interaction without owning the suppression policy itself.

## Signals
The controller emits footer action requests and selected-row sync requests. QML handles only the visual consequences,
such as opening the context menu or focusing the selected row.
