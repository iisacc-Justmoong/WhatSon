# Regression Tests

This directory owns the in-repo automated regression suite for WhatSon.

The maintained regression surface combines root-level build gates with a Qt Test based C++ runtime suite under
`test/cpp/`.

The `test/CMakeLists.txt` entrypoint now stays intentionally thin and delegates the executable/test wiring to
`test/cpp/CMakeLists.txt`, so additional regression surfaces can be added without growing one monolithic test build
definition.

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
- Structured editor selection cleanup is now also locked at the C++ host-object layer, so block activation keeps
  emitting the selection-clear revision/retained-block contract that the QML delegates consume.
- Clipboard-image resource imports now also pin their synthesized asset-name policy in the C++ suite, so temporary
  placeholder names cannot regress back to a collision-prone fixed file name.
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
- The content-surface mode helper now also pins note-vs-resource editor routing, so a direct resource list model
  switches the center slot away from the note editor immediately.
- The dedicated resource editor QML is now also source-locked as a transparent viewer-only surface, so Resources
  hierarchy browsing cannot regress into a second background card or top/bottom metadata copy around the asset.
- The right detail column now also pins note-vs-resource panel routing, so the resources hierarchy mounts its own
  dedicated viewmodel/view pair instead of reusing the note-detail surface.
- Rapid note switches now also pin note-local gutter geometry invalidation, so the editor clears stale minimap/gutter
  line caches on note entry and forces a fresh layout-cache pass before reusing line-number coordinates.
- `ResourceBitmapViewer` now also pins bitmap-preview projection for the dedicated resource editor, so image resources
  and unsupported image-like formats expose stable viewer/open-target state to QML.
- Page/print paper-palette routing now also pins both the HTML renderer and the structured editor QML wiring, so
  bright screen-only text colors cannot leak onto the white paper surface in `Page` / `Print` mode.
- Developer quality gates now also resolve their QML/C++ file lists from the repository root, so `whatson_qmllint`
  and `whatson_clang_tidy` no longer degrade into empty-input no-op runs.
