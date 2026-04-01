# `src/app/qml/view/panels/NoteListItem.qml`

## Status
- Documentation phase: scaffold generated from the live source tree.
- Detail level: structural placeholder prepared for a later deep pass.

## Source Metadata
- Source path: `src/app/qml/view/panels/NoteListItem.qml`
- Source kind: QML view/component
- File name: `NoteListItem.qml`
- Approximate line count: 320

## QML Surface Snapshot
- Root type: `Item`

### Object IDs
- `noteListItem`
- `noteHoverHandler`
- `foldersRow`
- `tagsRow`

### Required Properties
- None detected during scaffold generation.

### Signals
- `viewHookRequested`

## Recent Updates
- Added `pragma ComponentBehavior: Bound` to enforce bound delegate id access for nested metadata rows.
- Folder/tag metadata `Repeater` delegates now declare `required property var modelData` and read
  `folderLabelRow.modelData` / `tagLabelRow.modelData` explicitly.
- Resources rows no longer reuse this component; `ListBarLayout.qml` now mounts dedicated
  `ResourceListItem.qml` when the bound model exposes `currentResourceEntry`.

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
