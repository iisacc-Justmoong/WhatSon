# `src/app/qml/window/OnboardingContent.qml`

## Status
- Documentation phase: scaffold generated from the live source tree.
- Detail level: structural placeholder prepared for a later deep pass.

## Source Metadata
- Source path: `src/app/qml/window/OnboardingContent.qml`
- Source kind: QML view/component
- File name: `OnboardingContent.qml`
- Approximate line count: 619

## QML Surface Snapshot
- Root type: `Item`

## Current Notes
- Desktop/mobile onboarding chrome now routes its visible geometry through LVRS metrics:
  - icon, close affordance, action widths, content widths, and panel widths use `LV.Theme.scaleMetric(...)`
  - spacing and insets use `LV.Theme.gap16/gap24`
  - outer surface radius uses `LV.Theme.radiusXl * 2`
- The close glyph no longer depends on fixed `5/11/1.6px` canvas coordinates; it derives its stroke path from the
  live button size so LVRS UI scaling keeps the icon centered and crisp.

### Object IDs
- `root`
- `createHubDialog`
- `createHubDirectoryDialog`
- `selectHubDialog`
- `selectHubFileDialog`
- `windowFrame`
- `closeColumn`
- `closeButton`
- `closeMouseArea`
- `rightPanel`
- `appPanel`
- `appPanelContent`
- `appPanelColumn`
- `createHubAction`
- `selectHubAction`
- `mobileAppColumn`
- `actionLink`
- `actionText`
- `actionMouseArea`

### Required Properties
- `index`
- `modelData`

### Signals
- `completed`
- `createFileRequested`
- `dismissRequested`
- `requestWindowMove`
- `selectFileRequested`
- `viewHookRequested`
- `triggered`

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
