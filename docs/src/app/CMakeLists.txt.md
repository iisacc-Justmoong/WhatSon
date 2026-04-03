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
- The app target now declares `Qt6::Pdf` and `Qt6::PdfQuick` as first-class dependencies because `ContentsResourceViewer.qml` imports `QtQuick.Pdf` for direct `.wsresource` PDF rendering.
- iOS keeps `QT_QML_MODULE_NO_IMPORT_SCAN` enabled for the clean Xcode export flow, so the app now carries an explicit static QML plugin closure instead of relying on top-level imports alone.
- That closure includes the QML runtime foundation (`Qt6::qmlplugin`, `Qt6::modelsplugin`, `Qt6::workerscriptplugin`), the controls/dialog implementation chain (`Qt6::qtquicktemplates2plugin`, `Qt6::qtquickcontrols2implplugin`, `Qt6::qtquickcontrols2basicstyleimplplugin`, `Qt6::qtquickcontrols2iosstyleimplplugin`, `Qt6::qtquickdialogs2quickimplplugin`), and the feature-facing plugins already used by app QML.
- `QtQuick.Pdf` is not self-contained on iOS in this static-plugin setup: `PdfMultiPageView.qml` also imports `QtQuick.Shapes`, so `Qt6::qmlshapesplugin` must stay in the same manual plugin link set or the QML engine aborts during startup.
- This defensive list mirrors the transitive plugin closure hidden behind `QT_IS_PLUGIN_GENEX` inside Qt's imported plugin targets, which makes future `module \"...\" is not installed` regressions less likely when `PdfQuick`, dialogs, or control styles pull nested QML imports.

## Authoring Notes For Next Pass
- Read the real implementation and adjacent headers before replacing this scaffold.
- Document concrete signals, slots, invokables, persistence side effects, and LVRS/QML bindings where applicable.
- Cross-link this file with peer modules in the same directory once the detailed pass begins.
