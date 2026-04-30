# `src/app/qml/view/panels/navigation/NavigationModeBar.qml`

## LVRS Token Notes
- Compact mode spacing uses `LV.Theme.gap8` / `LV.Theme.gapNone`.

## Status
- Documentation phase: scaffold generated from the live source tree.
- Detail level: structural placeholder prepared for a later deep pass.

## Source Metadata
- Source path: `src/app/qml/view/panels/navigation/NavigationModeBar.qml`
- Source kind: QML view/component
- File name: `NavigationModeBar.qml`
- Approximate line count: 84

## QML Surface Snapshot
- Root type: `LV.HStack`

### Object IDs
- `modeBar`
- `modeCombo`
- `modeContextMenu`

### Required Properties
- None detected during scaffold generation.

### Signals
- `viewHookRequested`

## Recent Updates
- Context-menu `selectedIndex` now resolves through `modeBar.navigationModeViewModel` to keep
  nested menu bindings explicitly scoped to the root id.
- Mode label and context-menu fallback now default to `View` (`selectedIndex: 0`) when the bound
  navigation mode viewmodel is not yet resolved.
- Compact/mobile combo sizing now routes through `compactComboWidth` (`97` scaled px) when
  `showLabel: false`, matching the mobile dual-combo navigation frame (`174:5689`).
- Popup/menu metrics now use `comboMenuYOffset` (`LV.Theme.gap2`) and `comboContextMenuWidth`
  (`LV.Theme.buttonMinWidth + LV.Theme.gap24 + LV.Theme.gap8`) instead of fixed literals.

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
