# `src/app/viewmodel/hierarchy/tags/TagsHierarchyModel.hpp`

## Status
- Documentation phase: scaffold generated from the live source tree.
- Detail level: structural placeholder prepared for a later deep pass.

## Source Metadata
- Source path: `src/app/viewmodel/hierarchy/tags/TagsHierarchyModel.hpp`
- Source kind: C++ header
- File name: `TagsHierarchyModel.hpp`
- Approximate line count: 87

## Extracted Symbols
- Declared namespaces present: no
- QObject macro present: yes

### Classes and Structs
- `TagsHierarchyItem`
- `TagsHierarchyModel`

### Enums
- `Role`

## Current Implementation Notes
- `TagsHierarchyItem` now carries a dedicated `iconName` field for LVRS hierarchy presentation.
- `tagsHierarchyIconName(...)` defines the canonical fallback tag glyph as `vcscurrentBranch`, matching the icon
  already used for note tag metadata chips/cards elsewhere in the app.
- `TagsHierarchyModel::Role` now exports that presentation value through the `iconName` role so any row consumer that
  binds directly to the Qt model can render the same tag icon consistently.

## Intended Detailed Sections
- Responsibility and business role
- Ownership and lifecycle
- Public API or externally observed bindings
- Collaborators and dependency direction
- Data flow and state transitions
- Error handling and recovery paths
- Threading, scheduling, or UI affinity constraints when relevant
- Extension points, invariants, and known complexity hotspots
- Test coverage and missing verification

## Authoring Notes For Next Pass
- Read the real implementation and adjacent headers before replacing this scaffold.
- Document concrete signals, slots, invokables, persistence side effects, and LVRS/QML bindings where applicable.
- Cross-link this file with peer modules in the same directory once the detailed pass begins.
