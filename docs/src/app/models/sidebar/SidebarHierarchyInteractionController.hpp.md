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
- `captureExpansionState(...)`, `modelWithPreservedExpansion(...)`, `handleExpansionSignal(...)`, and
  `requestChevronExpansion(...)` keep row expansion state and mutation routing out of QML.
- `beginActivationAttempt(...)`, `activationAttemptCurrent(...)`, and `shouldSuppressActivation(...)` let the view defer
  activation safely after chevron interaction without owning the suppression policy itself.

## Signals
The controller emits expansion-state and binding-change signals only. Footer action signals and selected-row sync relay
signals are intentionally absent; `SidebarHierarchyView.qml` owns footer dispatch and schedules post-expansion visual
sync with `Qt.callLater(...)`.
