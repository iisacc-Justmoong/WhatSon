# `src/app/viewmodel/sidebar/SidebarHierarchyViewModel.hpp`

## Role
This header defines the QObject that represents sidebar hierarchy routing state.

It does not own the hierarchy data itself. Instead, it owns:
- which hierarchy domain is active
- how that active domain maps to a concrete hierarchy viewmodel
- how that active domain maps to a concrete note-list model

## Public Surface
- `activeHierarchyIndex`
- `activeHierarchyViewModel`
- `activeNoteListModel`
- `hierarchyViewModelForIndex(...)`
- `noteListModelForIndex(...)`
- mutable wiring for `selectionStore` and `viewModelProvider`

## Data Sources
- `ISidebarSelectionStore` stores the selected hierarchy index.
- `IHierarchyViewModelProvider` resolves the viewmodel and note-list model for a normalized domain index.

## Architectural Constraint
This type is a coordinator. It should not absorb domain-specific hierarchy mutation logic. That belongs in the concrete hierarchy viewmodel or a bridge layered above it.
