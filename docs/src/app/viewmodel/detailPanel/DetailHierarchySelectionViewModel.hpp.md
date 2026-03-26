# `src/app/viewmodel/detailPanel/DetailHierarchySelectionViewModel.hpp`

## Responsibility
`DetailHierarchySelectionViewModel` is a detail-panel-local selector state object.
It mirrors hierarchy entries from a source hierarchy viewmodel, but it owns its own `selectedIndex` so the detail panel never mutates or follows sidebar hierarchy selection implicitly.

## Public Contract
- Exposes `hierarchyModel` for QML combo/context-menu binding.
- Exposes `selectedIndex` as a detail-panel-local selection slot.
- Exposes `itemCount` for simple availability checks.
- Accepts a `sourceViewModel` QObject that must publish `hierarchyModel`, `selectedIndex`, `hierarchyModelChanged()`, and `selectedIndexChanged()`.

## Invariants
- Source hierarchy data is copied, not shared as mutable selection state.
- Initial selector state can seed itself from the source selection once.
- Subsequent source selection changes are mirrored only when they come from the injected detail selector source object.
- Subsequent source hierarchy data changes refresh the copied entries while preserving the local selection by stable entry key when possible.
- The raw sidebar hierarchy viewmodels are still not used as mutable selector sources directly; they remain option providers behind the file-backed selector source object.
