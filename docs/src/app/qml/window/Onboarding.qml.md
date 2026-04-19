# `src/app/qml/window/Onboarding.qml`

## Status
- Documentation phase: scaffold generated from the live source tree.
- Detail level: structural placeholder prepared for a later deep pass.

## Source Metadata
- Source path: `src/app/qml/window/Onboarding.qml`
- Source kind: QML view/component
- File name: `Onboarding.qml`
- Approximate line count: 119

## QML Surface Snapshot
- Root type: `Window`

## Current Notes
- Desktop/mobile onboarding window design size and minimum size baselines now route through `LV.Theme.scaleMetric(...)`
  instead of raw `542/867/420/620/762/470px` literals.
- The outer window still delegates all visual chrome to `OnboardingContent.qml`; this file only owns window sizing,
  recentering, modality, and route-to-content wiring.
- Ordinary desktop startup now depends on this wrapper again: when no persisted hub can be mounted, `Main.qml` keeps the
  workspace shell alive in the background and raises this separate modal onboarding window on top.
- `Onboarding.qml` also remains the explicit `whatson --onboarding-only` shell, so desktop startup recovery and the
  dedicated onboarding-only entrypoint share the same window owner.

### Object IDs
- `root`

### Required Properties
- None detected during scaffold generation.

### Signals
- `createFileRequested`
- `dismissed`
- `selectFileRequested`
- `viewHookRequested`

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
