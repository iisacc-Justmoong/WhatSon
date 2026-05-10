# App Architecture

## Desktop Workspace
The root `ApplicationWindow` `panelBackground01` canvas remains the only broad desktop background surface.

The active workspace route keeps the desktop shell layout: status bar, navigation bar, body layout, hierarchy sidebar,
note list, content slot, and detail panel. The content slot mounts `ContentViewLayout.qml`, and that surface mounts only
`Gutter.qml`, `TextEditor.qml`, and `Minimap.qml`.

## Mobile Shell
The mobile shell remains mounted for adaptive/mobile layouts. Its editor route uses the same `ContentViewLayout.qml`
surface and forwards active-note state so the selected note body file is edited while keeping the existing route
scaffold, hierarchy page, note-list page, and detail page chrome.

## Root Ownership
`Main.qml` owns startup routing, onboarding presentation, the restored workspace chrome, and render-quality resize
policy. It does not route TextEditor mutations through parser/projection/rendering/persistence backends.

Runtime objects now arrive from `WhatSonQmlContextBinder` as direct LVRS context-object bindings. QML does not use a
view-model layer or a `LV.Controllers`/`LV.ViewModels` registry for runtime lookup.

The binder no longer publishes an editor view-mode controller. The active editor surface is the LVRS `TextEditor`
composition path with `filePath` bound to `NoteActiveStateTracker.activeNoteBodyPath`.

## View Behavior Ownership
QML owns behavior that is local to a rendered view: button dispatch, menu opening/closing, pointer hit-tests, transient
visual state, focus presentation, and short callback/signal coalescing used to keep an LVRS surface from double-firing.

Do not add a C++ controller signal round-trip for those view-local behaviors. If a view action needs to mutate domain
state, the QML surface should call the already exposed narrow model/controller/bridge API directly, leaving persistence,
parsing, scheduling, and domain mutation policy in the owning model layer.

The boundary is intentionally narrow: one-tick UI follow-up, disabled-button guards, and event-to-function dispatch can
live in QML, while state preservation, rollback, multi-call consistency, parser/renderer output, and geometry/line
metrics remain C++ responsibilities.

## Routed Workspace
The current workspace route keeps the previous hierarchy/list/detail/mobile route structure. Onboarding still uses the
LVRS page stack, and `/` resolves to the restored workspace shell.

## Hierarchy Contract
The active workspace mounts the hierarchy surface as part of the existing layout. This is shell/navigation chrome, not a
TextEditor backend.

The detail panel no longer binds itself to the sidebar through ad-hoc lambda wiring in `main.cpp`.
Composition-root code now links those modules through `IActiveHierarchyContextSource` plus
`DetailPanelCurrentHierarchyBinder`, keeping sidebar/detail-panel coupling behind an explicit coordinator object.

Library system buckets now emit `draggable`, `dragAllowed`, `movable`, and `dragLocked`, and the hierarchy row baseline still resolves to a `20px` LVRS item height.

`MobileHierarchyPage.qml` remains part of the adaptive/mobile workspace mount path and no longer forwards editor
view-mode controller state.

## Control Surfaces
The compact control menu anchors from the trigger's bottom-right point. On the mobile hierarchy/control route, that trigger uses the `toolwindowtodo` glyph plus the built-in LVRS chevron instead of the old project-structure icon, and the trigger keeps the Figma `2 / 4 / 2 / 2` padding contract.

Action-only control entries disable the default LVRS shortcut placeholder column so icon-only mobile actions keep their full available label width.

Desktop navigation edge actions and calendar actions remain part of the restored shell. They must not reintroduce
TextEditor parser/projection/rendering/persistence ownership.

Calendar content surfaces now consume a shared `CalendarBoardStore`, and board mutations are modeled as explicit
`date + time` payload APIs (`addEvent`, `addTask`) so future reminder/event assignment can stay consistent across
day/week/month/year surfaces.

## Hub Sync
`WhatSonHubSyncController` is a filesystem watcher plus debounce/timer coordinator for the mounted `.wshub`.

It no longer listens to generic app activation, pointer press, or touch events. Runtime reloads now depend on observed
filesystem changes and local-mutation acknowledgements only, which keeps hub sync separate from navigation and input
policy.

The controller now computes the hub signature and watcher path set in a single recursive observation pass, so one sync
hint maps to one filesystem walk instead of one walk for hashing plus another for watcher coverage.

## Runtime Index Pipeline
`WhatSonLibraryIndexedState` is the backend projection boundary for library note indexing. It owns the canonical `all`,
`draft`, and `today` note collections so controllers do not each rebuild those derived buckets themselves.

`WhatSonRuntimeParallelLoader.cpp` now uses LVRS `BootstrapParallel` for requested domain loads and derives bookmarks
from the shared library snapshot when both domains are mounted. That keeps the runtime bootstrap on one library index
traversal instead of reparsing the same `.wshub` tree for the bookmarks domain.
