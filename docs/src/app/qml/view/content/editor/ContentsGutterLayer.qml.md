# `src/app/qml/view/content/editor/ContentsGutterLayer.qml`

## Status
- Documentation phase: scaffold generated from the live source tree.
- Detail level: structural placeholder prepared for a later deep pass.

## Source Metadata
- Source path: `src/app/qml/view/content/editor/ContentsGutterLayer.qml`
- Source kind: QML view/component
- File name: `ContentsGutterLayer.qml`
- Approximate line count: 79

## QML Surface Snapshot
- Root type: `Rectangle`

### Object IDs
- `gutterLayer`
- `lineNumberViewport`

### Required Properties
- `modelData`
- `modelData`

### Signals
- None detected during scaffold generation.

## LVRS/QML Standard Alignment
- Declares `pragma ComponentBehavior: Bound` for strict delegate scope.
- Gutter marker delegates now map model payload through `required property var modelData` plus an ID-qualified
  `markerSpec` projection (`gutterMarker.modelData`), avoiding unqualified delegate context reads.
- Optional Y/height resolver callbacks now flow through `resolveNumericResolverValue(...)` before projection so var
  callback dispatch remains explicit and lint-safe.

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
