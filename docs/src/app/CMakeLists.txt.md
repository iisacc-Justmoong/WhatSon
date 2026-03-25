# `src/app/CMakeLists.txt`

## Status
- Documentation phase: scaffold generated from the live source tree.
- Detail level: structural placeholder prepared for a later deep pass.

## Source Metadata
- Source path: `src/app/CMakeLists.txt`
- Source kind: CMake build definition
- File name: `CMakeLists.txt`
- Approximate line count: 625

## Build Surface Snapshot
- `find_package`
- `find_package`
- `qt_policy`
- `if`
- `get_target_property`
- `if`
- `set`
- `foreach`
- `if`
- `continue`
- `endif`
- `list`
- `endforeach`
- `set_property`
- `endif`
- `endif`
- `file`
- `list`
- `if`
- `list`
- `else`
- `list`
- `endif`
- `if`
- `message`

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

## Current Notes
- All `src/app/qml/**/*.qml` files are still globbed into the `WhatSon.App` QML module, so adding a new desktop helper window under `qml/window/` automatically exports it to `loadFromModule(...)`.
- Desktop trial builds pull in the dedicated trial activation sources from `src/extension/trial` and define `WHATSON_IS_TRIAL_BUILD=1` for the app target.
- Android and iOS builds intentionally skip the trial sources because the mobile app does not participate in the desktop trial flow.
- On Apple desktop trial builds, the app target also links the `Security` framework because the trial secure-store implementation uses the host keychain.

## Authoring Notes For Next Pass
- Read the real implementation and adjacent headers before replacing this scaffold.
- Document concrete signals, slots, invokables, persistence side effects, and LVRS/QML bindings where applicable.
- Cross-link this file with peer modules in the same directory once the detailed pass begins.
