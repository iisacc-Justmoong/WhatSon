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
- `HierarchyControllerProvider`
- `SidebarHierarchyController`
- `NoteListModelContractBridge`
- `NavigationModeController`
- `EditorViewModeController`
- `OnboardingRouteBootstrapController`
- `WhatSonCronExpression`
- `WhatSonAsyncScheduler`
- `WhatSonUnixTimeAnalyzer`
- `ContentsResourceTagTextGenerator`
- `WhatSonNoteFolderSemantics`
- `ContentsStructuredDocumentCollectionPolicy`
- `ContentsStructuredDocumentHost`
- `ContentsStructuredDocumentMutationPolicy`
- `WhatSonClipboardResourceImportFileNamePolicy`
- `ContentsEditorSessionController`
- `ContentsNoteManagementCoordinator`
- `ContentsEditorSurfaceModeSupport`
- `ContentsTextFormatRenderer`
- `ResourceDetailPanelController`
- `ResourceBitmapViewer`
- CMake wiring for the local `iiXml` and `iiHtmlBlock` package dependencies
- The iiXml-only `.wsnbody` explicit-block parser path, the iiHtmlBlock per-token display-block projection, and the
  shared note-package iiXml support layer used by `.wsnhead` and `.wsnbody` readers

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

- Source-tree policy coverage locks minimap model placement under `src/app/models/editor/minimap`, preventing legacy
  root or nested display chrome directories from returning.
- Structured editor selection cleanup is now also locked at the C++ host-object and QML routing layers, so focus
  activation emits the selection-clear revision/retained-block contract that QML delegates consume, while same-block
  cursor movement uses a cursor-only host path and keeps native desktop/iOS text selection intact.
- Editor shortcut-surface gating now also treats every focused body `LV.TextEditor.editorItem` as the native keyboard
  owner, so platform text-navigation and selection chords remain OS/Qt behavior instead of being shadowed by ordinary app shortcut
  handling. The explicit tag-management surface remains enabled outside composition so inline formatting shortcuts still
  write RAW tags while editing.
- Focused editor tag-management coverage now also pins direct RAW body-tag shortcuts for agenda, callout, and break
  insertion, plus press-phase right-click context-menu requests, so formatting commands do not depend on a later
  platform-specific click signal.
- Structured tag-management coverage now also requires the RAW mutation handler to accept inline-format/body-tag
  mutations before those commands report success, and verifies that explicit tag mutations bind the selected note
  session and request immediate persistence.
- Inline editor resource-frame coverage now verifies both pointer selection and collapsed cursor movement: image
  resource frames select as one atomic block, and the native caret cannot remain inside the frame's logical placeholder
  line even when the resource is the first block in the note.
- Inline resource HTML coverage now pins zero-line-height frame paragraphs with top-aligned frame images, and the QML
  gutter regression checks that the next row y matches `resourceRow.y + rendered frame height`. The C++ geometry
  regression also covers hidden TextEdit line boxes whose rectangle height differs from the next row's base-y advance
  and probe rows whose measured top is still inside the rendered resource frame.
- Clipboard-image resource imports now also pin their synthesized asset-name policy in the C++ suite, so temporary
  placeholder names cannot regress back to a collision-prone fixed file name.
- Structured QML editor checks now also pin the native input contract for every host: focused stale text echo is rejected,
  live block/task/callout edit baselines are preserved, and shortcut/key interception stands down while the
  OS keyboard owns the session.
  The same source checks pin pending cursor/surface restore behavior so neither path writes through native composition.
- Inline-format editor checks now also lock the C++/QML helper-controller path so programmatic sync policy and
  tag-management shortcut routing stay tied to the `LV.TextEditor.editorItem` native text surface.
- Inline-format editor checks now also lock rendered-mode projection catch-up: native `LV.TextEditor.text` is refreshed
  only through the controller programmatic-sync path, and `ContentsStructuredDocumentFlow.qml` holds the last ready
  logical projection instead of exposing RAW inline tags while source and projection are briefly out of step.
- Inline tag-insertion regression now also pins re-formatting over existing inline style tags: selections that land
  inside hidden RAW tag tokens, or include only one side of an existing wrapper, are normalized before the next RAW
  mutation so malformed `<highligh<...` source cannot surface in the editor.
- Structured QML editor checks now instantiate `ContentsDocumentTextBlock.qml` with RAW inline style tags and verify that
  the live editor receives rendered overlay HTML while its editable plain-text buffer stays tag-free.
- Structured QML editor checks now also lock the custom-input policy: ordinary editor input has no QML key handlers,
  markdown list shortcuts/continuation helpers stay absent, and only tag-management commands may use host shortcut
  surfaces or selected atomic-block key handling.
- Hierarchy controller switching now also pins QObject ownership at the sidebar/selection-bridge boundary, so switching
  `Resources -> Library` cannot hand member-owned C++ models to the QML garbage collector.
- Hierarchy-driven note-list rebinding is now also locked at the bridge layer, so swapping only the active hierarchy
  controller still replaces the effective note-list model immediately for desktop/mobile list surfaces.
- The same bridge coverage now also pins explicit note-list-model overrides, so `Resources -> Library` toolbar
  switches keep folder/tag note-list metadata aligned with the shared active list model instead of lingering on the
  previous domain snapshot.
- The regression build wiring now also follows the `src/app/models/*` move for agenda/calendar/callout backends, so
  source-tree reshuffles under the new models root remain covered by the maintained `build/` verification flow.
- RAW note hyperlink promotion now also has regression coverage, so committed/pasted URLs, `.wsnbody` round-trips,
  closing-tag cursor offsets, and RichText external-link activation stay aligned on the new `<weblink href="...">`
  contract.
- Note-body persistence now also locks inline style projection from stored RAW tags to rendered HTML, so a source run
  like `<bold>Al<italic>pha</italic></bold><italic> Beta</italic>` cannot reappear in the editor projection as literal
  XML text.
- Library note-list preview coverage now also locks that same RAW/visible split for note cards: `primaryText` hides
  inline source tags while `bodyText` remains the editor bootstrap RAW source.
- `.wsresource` package support now also has regression coverage for annotation-canvas generation, so package metadata
  round-trips the new `annotationPath` and newly created resource packages keep a transparent `annotation.png`.
- Folder-path semantics now also lock escaped literal-slash handling plus `Folders.wsfolders` parser migration, so a
  library folder label like `Marketing/Sales` cannot regress into an accidental parent/child hierarchy split.
- Sidebar hierarchy expansion now also pins the LVRS chevron signal path, so a folder row's right chevron forwards the
  `onListItemExpanded(...)` callback into `SidebarHierarchyInteractionController`, which issues one
  `setItemExpanded(...)` request while stale model refreshes still cannot overwrite the newly chosen expanded/collapsed
  state.
- Library hierarchy coverage now also pins that accent root folders with visible chevrons remain expandable; CRUD
  protection cannot make the chevron interface a no-op.
- Hierarchy interaction bridge coverage also locks the post-startup QML binding path: after the architecture policy
  lock, the QML-created interaction bridge must still bind and rebind the active hierarchy controller so chevron
  expansion can commit instead of rolling back.
- QML surface policy coverage now also pins the chevron-only pointer surface in `SidebarHierarchyView.qml`: it must
  accept only resolved chevron-slot left clicks and route them to the same C++ expansion commit path.
- Sidebar footer actions now pin both the legacy `LV.ListFooter` config-callback route and the `onButtonClicked` signal
  route, with QML direct dispatch and same-turn coalescing so create, delete, and context-menu clicks stay live without
  depending on a controller signal round-trip.
- Architecture policy coverage now pins the broader view-behavior contract: view-local behavior belongs in QML, while
  model/controller layers remain responsible for domain mutation, persistence, parsing, scheduling, and shared policy.
- The C++ regression executable links `SidebarHierarchyInteractionController.cpp` with the sidebar suite, so expansion
  state, rollback, and activation-suppression assertions exercise the real controller implementation while footer
  dispatch is asserted to remain outside that C++ policy object.
- Sidebar footer toolbar order is source-locked as add folder, delete selected folder, then open context menu, with the
  third slot using the LVRS more/menu icon instead of a settings glyph.
- Sidebar tree context-menu coverage now locks `Expand All` and `Collapse All` entries to
  `HierarchyInteractionBridge.setAllItemsExpanded(...)`, so full-tree folding stays a menu action rather than a row or
  footer-only mutation.
- The same coverage forbids the old `hierarchyViewOptionsMenuItems` alias; tree menu behavior must stay absorbed into
  the current `hierarchyTreeContextMenuItems` object.
- Hierarchy drag/drop bridge coverage now exercises the concrete note-list-item drop contract: dragged note ids are
  normalized, deduplicated, checked against the folder target, and assigned through `assignNotesToFolder(...)`.
- Sidebar hierarchy rename now also prefers the escaped folder-path id over a naive display-label split, so the same
  `Marketing/Sales` folder still seeds the inline rename editor as one literal label instead of collapsing to `Sales`.
- Detail-panel folder assignment now also reuses an existing escaped folder path instead of recreating
  `Marketing/Sales` as a fake nested hierarchy during `Folders.wsfolders` writes.
- Hierarchy selection normalization is now also locked in the C++ suite, so entering a populated hierarchy keeps the
  effective selection on the first visible row instead of falling back to an invisible `-1` / "show everything"
  state.
- Resources and progress now each pin their domain fallback semantics in the C++ suite, so the initial active UI row
  (`Image`, `First draft`) remains identical to the list filter applied by the corresponding controller.
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
  dedicated controller/view pair instead of reusing the note-detail surface.
- Rapid note switches now also pin note-local minimap invalidation, so the editor clears stale minimap line caches on
  note entry and forces a fresh layout-cache pass before reusing cached coordinates.
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
- Minimap row painting now also pins document-geometry scaling and 8px horizontal padding in the QML regression suite,
  so the rail uses each row's real `contentY` / `contentHeight` instead of evenly spacing bars by `visualIndex`, and
  long notes keep a proportional minimap silhouette inside the padded inner width.
- Minimap interaction now also pins row-only visual chrome and direct delta scrolling in the QML regression suite, so
  vertical drags emit pixel deltas while the minimap stops rendering a viewport thumb or scrollbar indicator.
- Contents chrome layout tests now also pin `LV.Theme.gap8` top/bottom padding on both standalone `ContentsView.qml`
  and the runtime note editor chrome in `ContentViewLayout.qml`, so the gutter, editor viewport, and minimap share one
  vertical inset.
- Structured layout-cache commits now also pin a forced minimap snapshot refresh in the QML regression suite, so once
  `cachedLogicalLineEntries` lands after note parsing the minimap rebuilds from the same structured geometry instead of
  staying on the stale pre-layout snapshot.
- Minimap snapshot planning now also pins normalized logical-line snapshot entries in the C++ and QML regression
  suites, so diff planning compares renderer-normalized document lines instead of raw `.wsnbody` newline splits and
  structured notes keep the minimap aligned with the same document model the editor is rendering.
- Line-number rail metrics now also pin whole-document logical text row mapping, so gutter numbers follow the
  editor's logical line positions, including blank lines between hidden RAW tags, and consume measured y snapshots
  instead of spacing numbers by a simple row count. QML surface tests also pin the rail's `preferredWidth`, so the
  blank leading area before the line-number column stays half of the previous implicit button-width slack, and the
  active blue gutter bar follows cursor/selection source offsets.
- Line-number rail metrics now also pin independent row geometry: a tall resource frame keeps one gutter-line row
  instead of converting frame height into a multi-line gutter allocation, while later gutter rows keep their own plain
  logical y positions adjusted only by the rendered HTML resource image-height delta. The geometry provider ignores
  rendered overlay row coordinates for ordinary non-resource gutter rows, and refuses to use whole-document rendered
  `contentHeight` as a resource-row fallback. It also clamps probe rows reported inside an active resource frame to the
  frame bottom, preventing placeholder/probe internals from becoming visible gutter anchors.
- The shared inline-format editor now also pins the absence of pointer interception above the live `LV.TextEditor`,
  so mouse/touch selection, `Shift`-extended selection, and repeated Backspace/Delete remain OS/Qt-native. The same
  regression scans the QML source tree for forbidden input-method bridges and fallbacks, keeping IME query updates,
  candidate placement, and keyboard visibility on the OS/Qt text-editing path.
- The inline editor regression now also pins the explicit `LV.TextEditor` keyboard/selection flags used by note-body
  editors: focus-on-press, keyboard selection, pointer selection, persistent selection, unrestricted input-method hints,
  character-level mouse selection, and insert-mode editing.
- The inline editor regression now also pins WYSIWYG selection against the rendered HTML overlay: once `LV.TextEditor`
  owns a non-empty selection, the RichText overlay stays visible while the native editor owns the selection range and
  paints source glyphs transparent, so RAW resource tags cannot replace rendered resource frames.
- The inline editor regression now also pins resource-backed overlay stability during ordinary native editing: an
  iiHtmlBlock resource projection keeps the RichText frame pinned above the RAW buffer while keystrokes mutate the
  source, so transient composition/render turns cannot expose `<resource ... />` text.
- The inline editor regression now also pins exclusive caret painting: while rendered output is visible, only the
  projected WYSIWYG cursor is painted and the native logical cursor delegate remains hidden; disabling rendered output
  returns caret painting to the native editor path.
- The inline editor regression now also pins rendered-surface cursor placement: taps are resolved through visible
  logical text geometry and then mapped back through `logicalToSourceOffsets`, so hidden RAW tags do not determine the
  cursor location.
- The same rendered-cursor coverage now also pins collapsed Backspace against nested inline style tags: the native
  editor deletes the previous visible logical glyph and the resulting logical text delta is converted to RAW, so no
  deletion path can remove bytes from hidden `</italic>` / `</bold>` boundaries behind the visual caret.
- The inline editor regression now also pins the rendered-mode native edit surface as visible logical text, not RAW
  XML. Typing and Backspace at the native logical caret update the logical `LV.TextEditor` buffer and are immediately
  converted back into RAW `.wsnbody` source through `visibleTextMutationPayload(...)`.
- The inline editor regression now also pins the rendered geometry probe and transparent selection surface as plain
  `displayGeometryText`, not RichText HTML, so caret placement and pointer hit-testing use the same logical coordinate
  space as `ContentsEditorPresentationProjection`.
- Resource-backed pointer selection now also pins the atomic logical frame contract: `<resource ... />` maps to one
  U+FFFC logical placeholder, and pointer hits inside the rendered image frame snap to that resource block instead of
  selecting fake internal text positions.
- Empty inline-format wrapper coverage now also verifies that hidden-only RAW selections remain collapsed after
  projection to the native logical surface, so restoring `<highlight></highlight>`-only ranges cannot select the next
  visible character.
- The inline editor regression now also pins actual mouse-click cursor movement through both the inline editor and
  `ContentsStructuredDocumentFlow` host, including the projected caret rectangle, so collapsed pointer gestures cannot
  leave the visible cursor at the initial host-provided logical cursor position.
- The same click coverage now extends the selection with `Shift+Right` after mouse placement, so the native selection
  anchor starts from the clicked surface position instead of an initial RAW cursor fallback.
- The inline editor regression now also pins rendered pointer drag selection against cursor override regressions:
  collapsed pointer clicks may move the projected caret immediately, but non-empty pointer selections clear that
  override and hide the projected cursor while the selection model owns the highlighted range.
- Rendered pointer selection now also locks surface-to-RAW mapping through a transparent logical selection editor,
  including coordinate clamping inside the rendered text content bounds so line slack does not resolve to document end.
- The inline editor regression now also pins rendered pointer multi-click selection: double-click restores visible-line
  selection and triple-click restores visible-paragraph selection before mapping the selected logical range back to RAW
  source offsets.
- The inline editor regression now also pins the top-flush editor body contract: top inset and rendered-overlay top
  padding stay at zero, the legacy `LV.Theme.gap16` bottom body gap is added to the reported scrollable content height,
  and horizontal text-column margins are retained.
- The contents/editor QML regressions now also pin the scrollable document viewport: the center editor slot wraps the
  structured document flow in a `Flickable` and routes wheel events to that viewport, while bottom body breathing room
  remains owned by the inline editor surface.
- Contents QML placement is now locked under `src/app/qml/view/contents`: the standalone Figma
  `ContentsView/EditorView/Minimap` parts and the runtime editor host share that namespace, while
  `src/app/qml/contents` and `src/app/qml/view/content` are forbidden by the source-tree policy regression.
- The inline editor regression also source-locks the absence of live-text key handlers in
  `ContentsInlineFormatEditor.qml`, so ordinary navigation and selection chords stay with Qt/OS text editing.
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
- Architecture source-tree coverage now also rejects reintroducing `src/app/viewmodel`, MVVM naming, and LVRS
  controller/view-model registry lookup in `src/app`, keeping QML on direct context-object dependencies and behavior
  inside the owning model domains.
- Selected-note body snapshots now also distinguish unresolved sources from legitimate empty notes, so the editor
  session and note-mount placeholder only treat an empty body as authoritative after a direct source snapshot or a
  completed body load confirms that the selected note really resolves.
- Selection-bridge persistence completion now also pins signal ordering, so a successful same-note body save updates
  `selectedNoteBodyText` before `editorTextPersistenceFinished(...)` reaches the editor session. This prevents the
  presentation source from briefly falling back to a stale saved body while typing resumes after an idle save.
- Local dependency wiring now also has regression coverage: the root build must discover `iiXml` and `iiHtmlBlock`
  from cacheable local prefixes, and both the app and C++ regression targets must link their imported CMake targets.
- `.wsnhead` parsing now has iiXml migration coverage: `WhatSonNoteHeaderParser` must include iiXml, build its
  extraction from `TagParser::ParseAllDocumentResult`, and still preserve folders, tags, file statistics, progress,
  bookmark, and preset metadata.
- `.wsnbody` package reads now have iiXml migration coverage: `WhatSonLocalNoteFileStore` must locate `<body>` and
  first-resource thumbnail metadata through the iiXml document tree while preserving editor-facing RAW body source.
- Editor rendering now has XML-to-HTML-block pipeline coverage: `.wsnbody` explicit block spans must prefer iiXml,
  and `ContentsHtmlBlockRenderPipeline` must validate a renderer XML projection, convert it through iiHtmlBlock, and
  publish iiHtmlBlock display-block metadata on normalized HTML blocks.
- Editor rendering now also pins stored inline-style projection: RAW `<bold>` / `<italic>` tags must reach the
  editor surface as RichText style spans in `documentHtml`, `htmlTokens`, and `normalizedHtmlBlocks`, not as escaped
  literal XML-tag text.
- Runtime logging now has local dependency trace coverage: the app installs a Qt message filter that suppresses
  default-off `iiXml::*` debug chatter while preserving warnings and allowing `WHATSON_IIXML_TRACE_MODE=1` opt-in
  diagnosis.
- Editor-host coverage now verifies that `ContentsEditorPresentationProjection` republishes `htmlTokens` and
  `normalizedHtmlBlocks`, and that `ContentViewLayout.qml` forwards those backend-owned projection objects into
  `ContentsStructuredDocumentFlow.qml` for final rendering.

## 한국어

이 섹션은 위 README 내용을 한국어로 확인하기 위한 하단 요약이다.

- 대상: `Regression Tests` (`test/README.md`)
- 위치: `test`
- 역할: 이 파일은 해당 디렉터리나 모듈의 구조, 책임, 운영 규칙, 검증 기준을 설명하며 `LV.TextEditor` 기반 본문 입력 회귀를 포함한다.
- 현재 본문 입력 회귀에는 렌더 표면 마우스 클릭 후 `Shift+Right` 선택 앵커, 렌더 표면 드래그 selection,
  표면 selection-to-RAW 매핑 및 좌표 클램프 검증이 포함된다. 렌더 모드의 native edit surface가 RAW XML이
  아니라 visible logical text인지, 그리고 입력/Backspace가 즉시 RAW `.wsnbody` mutation으로 변환되는지도
  함께 검증한다.
- editor renderer 회귀에는 RAW `<bold>` / `<italic>` 인라인 스타일 태그가 literal XML 문자열이 아니라
  RichText 스타일 span으로 투영되는지 확인하는 케이스가 포함된다.
- 기준: 파일 경로, 명령, API 이름, 세부 변경 이력은 위 영어 본문을 원문 기준으로 유지한다.
- 변경 시: 위 영어 본문을 수정하면 이 한국어 하단 섹션도 함께 최신 상태로 맞춘다.
