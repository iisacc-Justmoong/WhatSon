# Regression Tests

This directory owns the in-repo automated regression suite for WhatSon.

The maintained regression surface combines root-level build gates with a Qt Test based C++ runtime suite under
`test/cpp/`.

The `test/CMakeLists.txt` entrypoint now stays intentionally thin and delegates the executable/test wiring to
`test/cpp/CMakeLists.txt`, so additional regression surfaces can be added without growing one monolithic test build
definition.

The maintained Qt Test suite is now split by regression subject instead of keeping every slot in one source file:

- `test/cpp/whatson_cpp_regression_tests.hpp` owns the shared fixture declarations, fake objects, and slot list.
- `test/cpp/whatson_cpp_regression_test_support.cpp` owns reusable helper implementations such as sandboxed settings,
  hub fixture creation, local note creation, JS loading, and source-file loading.
- `test/cpp/whatson_cpp_regression_tests_main.cpp` keeps the single Qt Test entrypoint and initializes a
  `QGuiApplication` with `QT_QPA_PLATFORM=offscreen` when no platform is already configured, so Quick item runtime
  checks can run beside the non-visual C++ assertions.
- `test/cpp/suites/*.cpp` owns the object-focused regression implementations. Each suite file now covers one object or
  one tightly coupled runtime seam instead of extending a monolithic translation unit.

- `whatson_build_regression` builds the maintained product binaries and the regression test executable in `build/`.
- `whatson_cpp_regression` runs the runtime C++ regression assertions only.
- `whatson_regression` is the default combined verification gate.

The C++ suite currently locks regression-sensitive runtime behavior for:

- `SelectedHubStore`
- `SidebarSelectionStore`
- `HierarchyViewModelProvider`
- `SidebarHierarchyViewModel`
- `NoteListModelContractBridge`
- `NavigationModeViewModel`
- `EditorViewModeViewModel`
- `OnboardingRouteBootstrapController`
- `WhatSonCronExpression`
- `WhatSonAsyncScheduler`
- `WhatSonUnixTimeAnalyzer`
- `ContentsResourceTagTextGenerator`
- `WhatSonNoteFolderSemantics`
- `ContentsStructuredDocumentCollectionPolicy`
- `ContentsStructuredDocumentHost`
- `ContentsStructuredDocumentMutationPolicy`
- `ContentsLogicalLineLayoutSupport.js`
- `WhatSonClipboardResourceImportFileNamePolicy`
- `ContentsEditorSessionController`
- `ContentsNoteManagementCoordinator`
- `ContentsEditorSurfaceModeSupport.js`
- `ContentsTextFormatRenderer`
- `ResourceDetailPanelViewModel`
- `ResourceBitmapViewer`

The suite avoids booting the full application shell or loading a hub package.

## Commands

Configure the repository in the standard build tree:

```bash
cmake -S . -B build
```

Run the maintained build gate:

```bash
cmake --build build --target whatson_build_regression -j
```

Run the default combined regression target:

```bash
cmake --build build --target whatson_regression -j
```

Run the same suite through `ctest`:

```bash
ctest --test-dir build --output-on-failure -L cpp_regression
```

## Current Focus

- The QML-side line-coordinate helper `ContentsLogicalLineLayoutSupport.js` is now regression-tested through
  `QJSEngine`, so gutter/minimap line placement keeps the block-local mapped Y contract even though the maintained
  runtime suite stays C++-driven.
- Structured editor selection cleanup is now also locked at the C++ host-object and QML routing layers, so focus
  activation emits the selection-clear revision/retained-block contract that QML delegates consume, while same-block
  cursor movement uses a cursor-only host path and keeps native desktop/iOS text selection intact.
- `ContentsDisplayView.qml` over-responsibility reduction is now source-locked by checking that gutter, minimap,
  mount-loading, exception, and resource-import alert chrome are composed through sibling view hosts instead of being
  owned directly by the root display host.
- The same coverage now pins `ContentsDisplaySurfacePolicy` as the C++ surface-selection contract used by
  note-selection focus restoration, so the root display host and selection/mount controller keep the parser-backed
  structured document flow as the only concrete note editor surface.
- `ContentsDisplaySurfacePolicy` is covered as the C++ surface-selection policy: selected notes use the structured
  document surface and the old whole-note inline loader stays disabled.
- Editor shortcut-surface gating now also treats every focused body `TextEdit` as the native keyboard owner, so platform
  text-navigation and selection chords remain OS/Qt behavior instead of being shadowed by ordinary app shortcut
  handling. The explicit tag-management surface remains enabled outside composition so inline formatting shortcuts still
  write RAW tags while editing.
- Clipboard-image resource imports now also pin their synthesized asset-name policy in the C++ suite, so temporary
  placeholder names cannot regress back to a collision-prone fixed file name.
- Structured QML editor checks now also pin the native input contract for every host: focused stale text echo is rejected,
  live block/task/callout edit baselines are preserved, and shortcut/key interception stands down while the
  OS keyboard owns the session.
  The same source checks pin pending cursor/surface restore behavior so neither path writes through native composition.
- Structured QML editor checks now instantiate `ContentsDocumentTextBlock.qml` with RAW inline style tags and verify that
  the live editor receives rendered overlay HTML while its editable plain-text buffer stays tag-free.
- Structured QML editor checks now also lock the custom-input policy: ordinary editor input has no QML key handlers,
  markdown list shortcuts/continuation helpers stay absent, and only tag-management commands may use host shortcut
  surfaces or selected atomic-block key handling.
- Hierarchy viewmodel switching now also pins QObject ownership at the sidebar/selection-bridge boundary, so switching
  `Resources -> Library` cannot hand member-owned C++ models to the QML garbage collector.
- Hierarchy-driven note-list rebinding is now also locked at the bridge layer, so swapping only the active hierarchy
  viewmodel still replaces the effective note-list model immediately for desktop/mobile list surfaces.
- The same bridge coverage now also pins explicit note-list-model overrides, so `Resources -> Library` toolbar
  switches keep folder/tag note-list metadata aligned with the shared active list model instead of lingering on the
  previous domain snapshot.
- The regression build wiring now also follows the `src/app/models/*` move for agenda/calendar/callout backends, so
  source-tree reshuffles under the new models root remain covered by the maintained `build/` verification flow.
- RAW note hyperlink promotion now also has regression coverage, so committed/pasted URLs, `.wsnbody` round-trips,
  closing-tag cursor offsets, and RichText external-link activation stay aligned on the new `<weblink href="...">`
  contract.
- `.wsresource` package support now also has regression coverage for annotation-canvas generation, so package metadata
  round-trips the new `annotationPath` and newly created resource packages keep a transparent `annotation.png`.
- Folder-path semantics now also lock escaped literal-slash handling plus `Folders.wsfolders` parser migration, so a
  library folder label like `Marketing/Sales` cannot regress into an accidental parent/child hierarchy split.
- Sidebar hierarchy rename now also prefers the escaped folder-path id over a naive display-label split, so the same
  `Marketing/Sales` folder still seeds the inline rename editor as one literal label instead of collapsing to `Sales`.
- Detail-panel folder assignment now also reuses an existing escaped folder path instead of recreating
  `Marketing/Sales` as a fake nested hierarchy during `Folders.wsfolders` writes.
- Hierarchy selection normalization is now also locked in the C++ suite, so entering a populated hierarchy keeps the
  effective selection on the first visible row instead of falling back to an invisible `-1` / "show everything"
  state.
- Resources and progress now each pin their domain fallback semantics in the C++ suite, so the initial active UI row
  (`Image`, `First draft`) remains identical to the list filter applied by the corresponding viewmodel.
- Startup hub selection now also pins the “no blueprint fallback” rule, so clearing the persisted selection cannot
  silently reopen a sample workspace during regression runs.
- Startup hub persistence now also pins the selection-URL/bookmark contract plus the iOS direct `.wshub` picker source
  wiring, so provider-backed mobile hub selection cannot regress back to a folder-only flow or a path-only startup
  restore.
- Startup presentation now also pins the "resolver success is not enough" rule, so a persisted hub that resolves to a
  path but still fails runtime load reopens onboarding on both desktop and mobile instead of leaving the app on a
  blank workspace shell.
- Hub creation now also pins the `WhatSonHubPackager` split, so package-root materialization and Apple package
  presentation remain separate from the scaffold files written by `WhatSonHubCreator`.
- The content-surface mode helper now also pins note-vs-resource editor routing, so a direct resource list model
  switches the center slot away from the note editor immediately.
- The dedicated resource editor QML is now also source-locked as a transparent viewer-only surface, so Resources
  hierarchy browsing cannot regress into a second background card or top/bottom metadata copy around the asset.
- The right detail column now also pins note-vs-resource panel routing, so the resources hierarchy mounts its own
  dedicated viewmodel/view pair instead of reusing the note-detail surface.
- Rapid note switches now also pin note-local gutter geometry invalidation, so the editor clears stale minimap/gutter
  line caches on note entry and forces a fresh layout-cache pass before reusing line-number coordinates.
- Empty selected notes now also pin one fallback structured `text-group` row in the source-locked QML regression
  checks, so selecting or creating a blank note still leaves a focusable body editor surface mounted.
- Structured parser output now also pins renderer-side interactive-stream normalization, so adjacent implicit text
  blocks collapse into one published `text-group` stream before the QML host consumes them and multi-block editing no
  longer depends on a duplicate client-side flatten pass.
- Structured document mutations now also pin editor-session write authority in the QML regression suite, so block-host
  edits immediately update `editorText`, mark local authority, and enter the normal RAW persistence path instead of
  disappearing when the user leaves the note.
- Structured document mutations now also pin current-source range validation in the QML regression suite, so stale
  mobile blur/dismiss events cannot splice an older block snapshot into the latest RAW note body and duplicate the
  text that was just saved.
- Minimap snapshots now also pin the resolved document presentation source in both the C++ and QML regression suites,
  so note-body snapshots populate the minimap immediately even before editor-session text becomes the active
  presentation authority.
- Minimap row painting now also pins document-geometry scaling in the QML regression suite, so the rail uses each
  row's real `contentY` / `contentHeight` instead of evenly spacing bars by `visualIndex`, and long notes keep a
  proportional minimap silhouette.
- Structured layout-cache commits now also pin a forced minimap snapshot refresh in the QML regression suite, so once
  `cachedLogicalLineEntries` lands after note parsing the minimap rebuilds from the same structured geometry instead of
  staying on the stale pre-layout snapshot.
- Minimap snapshot planning now also pins normalized logical-line snapshot entries in the C++ and QML regression
  suites, so diff planning compares renderer-normalized document lines instead of raw `.wsnbody` newline splits and
  structured notes keep the minimap aligned with the same document model the editor is rendering.
- The shared inline-format editor now also pins the absence of QML key/pointer interception above the live `TextEdit`,
  so mouse/touch selection, `Shift`-extended selection, and repeated Backspace/Delete remain OS/Qt-native. The same
  regression scans the QML source tree for forbidden input-method bridges and fallbacks, keeping IME query updates,
  candidate placement, and keyboard visibility on the OS/Qt `TextEdit` path.
- The inline editor regression now also pins the explicit Qt `TextEdit` keyboard/selection flags used by note-body
  editors: focus-on-press, keyboard selection, pointer selection, persistent selection, unrestricted input-method hints,
  character-level mouse selection, and insert-mode editing.
- The inline editor regression also source-locks the absence of live-text key handlers in
  `ContentsInlineFormatEditor.qml`, so ordinary navigation and selection chords stay with Qt/OS `TextEdit`.
- The unified display view now also pins blur-save behavior during native composition: blur flush returns instead of
  forcing RAW persistence after a fixed retry count while preedit text is still active.
- Inline structured resource cards now also pin block/card clipping in the QML regression suite, so a mobile image
  block cannot paint past its measured block bounds and overlay the following paragraph while the layout height
  catches up.
- The final editor HTML path now also pins canonical structured-source reparsing and corrected-source reuse in the
  runtime suite, so legacy `<hr>` aliases and self-closing structured tags cannot swallow following note text during
  the last render pass.
- `ResourceBitmapViewer` now also pins bitmap-preview projection for the dedicated resource editor, so image resources
  and unsupported image-like formats expose stable viewer/open-target state to QML.
- Page/print paper-palette routing now also pins both the HTML renderer and the structured editor QML wiring, so
  bright screen-only text colors cannot leak onto the white paper surface in `Page` / `Print` mode.
- Note-body mount coordination now also pins the retry/failure boundary between the selection bridge, the editor
  session, and the mounted document surface, so a selected note that still has no resolved body after one refresh pass
  shows the centered `No document opened` placeholder instead of leaving the chrome alive around an empty body layer.
- Mobile hierarchy navigation now also pins dismiss-style back targets in the C++ and source-locked QML regression
  suite, so leaving `/mobile/editor` returns to the current note list instead of falling through a raw hierarchy pop.
- The C++ regression executable now also links `MobileHierarchyNavigationCoordinator.cpp` with the mobile chrome
  suite, so the dismiss-plan assertions exercise the real coordinator implementation instead of failing at link time.
- Developer quality gates now also resolve their QML/C++ file lists from the repository root, so `whatson_qmllint`
  and `whatson_clang_tidy` no longer degrade into empty-input no-op runs.
- Project-local C++ headers now also lock repository-absolute include paths, so `app/...`, `extension/...`, and
  `test/...` remain stable even when domains move under `src/app/models` or regression suites move under `test/cpp`.
- Selected-note body snapshots now also distinguish unresolved sources from legitimate empty notes, so the editor
  session and note-mount placeholder only treat an empty body as authoritative after a direct source snapshot or a
  completed body load confirms that the selected note really resolves.
- `ContentsDisplayView` now also locks note-body mount coordination at the C++ and QML layers, so the center editor
  first retries a selected-note snapshot refresh, then mounts the resolved body into the editor session, and only
  then surfaces a centered `No document opened` placeholder instead of leaving gutter/minimap chrome visible beside an
  unmounted note surface.
- Selection-bridge persistence completion now also pins signal ordering, so a successful same-note body save updates
  `selectedNoteBodyText` before `editorTextPersistenceFinished(...)` reaches the editor session. This prevents the
  presentation source from briefly falling back to a stale saved body while typing resumes after an idle save.
