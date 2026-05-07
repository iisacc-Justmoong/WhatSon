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
- `primaryText` is a display-ready note preview supplied by the note-list model. The component must not parse or strip
  `.wsnbody` source tags itself; RAW/visible projection belongs to the model/file-note layer before QML receives the
  string.
- Resources rows no longer reuse this component; `ListBarLayout.qml` now mounts dedicated
  `ResourceListItem.qml` when the bound model exposes `currentResourceEntry`.
- Card padding, preview size, metadata spacing, and implicit row geometry now resolve from named `LV.Theme` spacing,
  icon, typography, and panel tokens instead of fixed pixel/color literals, so the note card follows LVRS
  desktop/mobile UI scale.
- The bookmark canvas glyph now derives its points from the live frame size, so the bookmark mark scales with the
  LVRS-sized icon frame instead of staying pinned to a `16px` path.
- Image placeholders now use the LVRS `strokeSoft` token instead of the previous raw gray fill.

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
