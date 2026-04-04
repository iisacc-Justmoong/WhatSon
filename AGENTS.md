# WhatSon Agent Operating Guide

## Objective

This repository implements a business-grade creative, branding, information, and knowledge text hub editor as an
LVRS-based Qt Quick application. This document defines stable operating rules so Codex works with consistent context
every turn.

## Technical Baseline

- UI stack: Qt 6.5+ QML + LVRS 1.0
- App entry: `src/app/main.cpp`
- App build definition: `src/app/CMakeLists.txt`
- Daemon entry: `src/daemon/main.cpp`
- Root build definition: `CMakeLists.txt`
- Primary QML root: `src/app/qml/Main.qml`
- Library hierarchy backend: `src/app/file/hierarchy/library`
- Library hierarchy model/viewmodel: `src/app/viewmodel/hierarchy/library/LibraryHierarchyModel.*`,
  `src/app/viewmodel/hierarchy/library/LibraryHierarchyViewModel.*`
- Library right-panel list model: `src/app/viewmodel/hierarchy/library/LibraryNoteListModel.*`
- Hub placement store: `src/app/file/hub/WhatSonHubPlacementStore.*`
- Tags depth provider: `src/app/file/hierarchy/tags/WhatSonHubTagsDepthProvider.*`
- Hub runtime store: `src/app/file/hub/WhatSonHubRuntimeStore.*`
- Runtime parallel bootstrap loader: `src/app/runtime/threading/WhatSonRuntimeParallelLoader.*`
- Tags runtime state store: `src/app/file/hierarchy/tags/WhatSonHubTagsStateStore.*`
- Tags hierarchy model/viewmodel: `src/app/viewmodel/hierarchy/tags/TagsHierarchyModel.*`,
  `src/app/viewmodel/hierarchy/tags/TagsHierarchyViewModel.*`
- Navigation mode state/viewmodel: `src/app/viewmodel/navigationbar/NavigationModeState.*`,
  `src/app/viewmodel/navigationbar/NavigationModeSectionViewModel.*`,
  `src/app/viewmodel/navigationbar/NavigationModeViewModel.*`
- Editor view mode state/viewmodel: `src/app/viewmodel/navigationbar/EditorViewState.*`,
  `src/app/viewmodel/navigationbar/EditorViewSectionViewModel.*`,
  `src/app/viewmodel/navigationbar/EditorViewModeViewModel.*`
- Sidebar selection store interface/impl: `src/app/store/sidebar/ISidebarSelectionStore.*`,
  `src/app/store/sidebar/SidebarSelectionStore.*`
- Sidebar hierarchy wiring interfaces/viewmodel: `src/app/viewmodel/sidebar/IHierarchyViewModelProvider.*`,
  `src/app/viewmodel/sidebar/HierarchyViewModelProvider.*`,
  `src/app/viewmodel/sidebar/SidebarHierarchyViewModel.*`
- Architecture policy lock and layer contract: `src/app/policy/ArchitecturePolicyLock.*`
- Runtime bootstrap: app startup resolves the first `blueprint/*.wshub` package and loads the critical startup domains
  in parallel worker threads via `WhatSonRuntimeParallelLoader`, then applies those snapshots to ViewModel objects on
  the main thread before the first workspace frame. Low-priority hierarchy domains may be deferred until first-frame
  idle turns or the first explicit sidebar activation.

## Automated Test Policy

- This repository does not maintain or operate an in-repo automated test suite.
- Python test scripts under `scripts/test_*.py` were removed and must not be reintroduced unless the user explicitly changes that policy.
- Do not add `tests/*`, `scripts/test_*.py`, or equivalent local test harnesses unless explicitly requested.
- Default task flow must not run project-level tests, smoke suites, or verification builds. Only perform verification when the user explicitly asks for it, and keep all generated artifacts under `build/`.

### Hierarchy ViewModel Ownership (Critical)

- Each hierarchy type must use its own dedicated ViewModel under `src/app/viewmodel/hierarchy/*`.
- Do not bind all hierarchy categories to a single shared ViewModel instance.
- Canonical mapping:
    - Library -> `LibraryHierarchyViewModel`
    - Projects -> `ProjectsHierarchyViewModel`
    - Bookmarks -> `BookmarksHierarchyViewModel`
    - Tags -> `TagsHierarchyViewModel`
    - Resources -> `ResourcesHierarchyViewModel`
    - Progress -> `ProgressHierarchyViewModel`
    - Event -> `EventHierarchyViewModel`
    - Preset -> `PresetHierarchyViewModel`
- Any hierarchy wiring change must preserve this one-type/one-ViewModel contract.
- Flat/shared hierarchy model abstraction is prohibited for runtime hierarchy ViewModel wiring.
- Keep model/support code isolated per domain directory in `src/app/viewmodel/hierarchy/<domain>/`.

## Architectural Direction

- Prefer Domain-Driven Development and Feature-Driven Development for new code, refactors, and file moves.
- Organize changes around domain or feature boundaries first, not around broad shared technical layers, unless the
  shared layer is already a stable cross-domain primitive.
- Keep each feature slice as end-to-end as possible: storage, runtime loading, service logic, viewmodel wiring, QML
  surface, and docs should stay close to the owning domain.
- Shared helpers are allowed only when multiple domains genuinely reuse them and when extracting them does not pull
  business rules out of the domain that owns those rules.

## Codex Init (`/init`) Procedure

Initialization guidance in this section is reference material only.
Do not execute this sequence during ordinary work unless the user explicitly asks for environment bootstrap or verification.

When `/init` is explicitly requested, initialization is considered complete when the following sequence succeeds.

1. Confirm LVRS package discoverability.
2. Run CMake configure.
3. Build app and daemon.
4. Run daemon healthcheck.
5. Run optional app offscreen smoke when needed.

Recommended commands:

```bash
cmake -S . -B build
cmake --build build --target WhatSon -j
cmake --build build --target WhatSon_daemon -j
./build/src/daemon/WhatSon_daemon --healthcheck
cmake --build build --target whatson_run_app
```

Normal host app and daemon builds must stay incremental and must not implicitly trigger the
dedicated `build-trial` packaging tree.
If a dedicated desktop trial package is explicitly required, opt in during configure and invoke
the nested packaging target manually:

```bash
cmake -S . -B build -DWHATSON_ENABLE_TRIAL_BUILD_MIRROR=ON
cmake --build build --target whatson_sync_trial_build
```

## Single Source of Truth for LVRS Integration

### CMake

The app target must keep this pattern.

```cmake
find_package(Qt6 6.5 REQUIRED COMPONENTS Quick QuickControls2)
find_package(LVRS CONFIG REQUIRED)

qt_add_executable(WhatSon main.cpp)
qt_add_qml_module(WhatSon
        URI WhatSon.App
        VERSION 1.0
        RESOURCE_PREFIX "/qt/qml"
        QML_FILES
        qml/Main.qml
)

lvrs_configure_qml_app(WhatSon)
```

### QML

All LVRS-based QML files use this import baseline.

```qml
import QtQuick
import LVRS 1.0 as LV
```

## Disallowed Changes

- Do not add manual `rpath`, manual `-F`, or manual framework paths for LVRS integration.
- Do not inject LVRS-specific import/plugin paths into `QQmlApplicationEngine` manually.
- Do not regress to a `QtQuick.Controls.Material`-centric UI as an LVRS replacement.
- Do not mix arbitrary Qt default widget styling into LVRS component screens.

## Allowed Exceptions

An exception is allowed only when all conditions are satisfied.

1. `find_package(LVRS)` and `lvrs_configure_qml_app()` fail in a reproducible way.
2. Failure logs are explicit and retained.
3. The change is reproducible and not a temporary workaround.
4. The reason and rollback condition are documented in PR/commit messages.

## Current UI Layout

- Root shell: `src/app/qml/Main.qml` (`LV.ApplicationWindow`)
- View directory (merged from legacy `shell` and `pages`):
    - `src/app/qml/view/top/StatusBarLayout.qml`
    - `src/app/qml/view/top/NavigationBarLayout.qml`
    - `src/app/qml/view/body/BodyLayout.qml`
    - `src/app/qml/view/mobile/MobilePageScaffold.qml` (shared mobile workspace scaffold for Figma VStack node `174:4987`, keeping the compact navigation bar and compact status/add-note bar mounted across pages)
    - `src/app/qml/view/mobile/pages/MobileHierarchyPage.qml` (Figma-driven routed mobile workspace page for hierarchy node `174:5026`, composed by `MobilePageScaffold.qml` plus the shared hierarchy panel)
    - `src/app/qml/view/panels/ListBarLayout.qml` (Figma-driven list bar panel for node `73:2635`)
    - `src/app/qml/view/panels/ListBarHeader.qml` (Figma-driven list bar header for node `134:3180`)
        - `src/app/qml/view/panels/NoteListItem.qml` (Figma-driven note item card for node `119:3028`)
        - `src/app/qml/view/panels/DetailPanelLayout.qml` (Figma-driven right panel wrapper for node `134:3212`)
        - `src/app/qml/view/panels/navigation/NavigationPropertiesBar.qml` (Figma frame `PropertiesBar`, node
          `147:3876`)
        - `src/app/qml/view/panels/navigation/NavigationInformationBar.qml` (Figma frame `InformationBar`, node
          `134:3138`)
        - `src/app/qml/view/panels/navigation/NavigationModeBar.qml` (Figma frame `NavigationMode`, node `147:3875`)
        - `src/app/qml/view/panels/navigation/NavigationEditorViewBar.qml` (Figma frame `EditorViewMoide`, node
          `148:3888`)
        - `src/app/qml/view/panels/navigation/NavigationApplicationViewBar.qml`
        - `src/app/qml/view/panels/navigation/NavigationApplicationEditBar.qml`
        - `src/app/qml/view/panels/navigation/NavigationApplicationControlBar.qml`
            - `src/app/qml/view/panels/detail/RightPanel.qml` (Figma frame `RightPanel`, node `134:3212`)
            - `src/app/qml/view/panels/detail/DetailPanel.qml` (Figma frame `DetailPanel`, node `134:3641`)
            - `src/app/qml/view/panels/detail/DetailPanelHeaderToolbar.qml` (Figma frame `DetailPanelHeaderToolbar`,
              node
              `134:3642`)
            - `src/app/qml/view/panels/detail/DetailContents.qml` (Figma frame `DetailContents`, node `134:3649`)
                - `src/app/qml/view/body/HierarchySidebarLayout.qml`
                    - `src/app/qml/view/panels/ContentViewLayout.qml`
                    - `src/app/qml/view/content/editor/ContentsDisplayView.qml`
                    - `src/app/qml/view/content/editor/ContentsGutterLayer.qml`
                    - `src/app/qml/view/content/editor/ContentsMinimapLayer.qml`
                    - `src/app/qml/view/content/editor/ContentsDrawerSplitter.qml`
                - `src/app/qml/view/panels/sidebar/SidebarHierarchyView.qml`
                - `src/app/qml/view/panels/sidebar/HierarchyViewLibrary.qml`
                - `src/app/qml/view/panels/sidebar/HierarchyViewProjects.qml`
                - `src/app/qml/view/panels/sidebar/HierarchyViewBookmarks.qml`
                - `src/app/qml/view/panels/sidebar/HierarchyViewTags.qml`
                - `src/app/qml/view/panels/sidebar/HierarchyViewResources.qml`
                - `src/app/qml/view/panels/sidebar/HierarchyViewProgress.qml`
                - `src/app/qml/view/panels/sidebar/HierarchyViewEvent.qml`
                - `src/app/qml/view/panels/sidebar/HierarchyViewPreset.qml`
- Shared components:
    - `src/app/qml/components/NavigationRail.qml`
    - `src/app/qml/components/MetricCard.qml`
    - `src/app/qml/components/InfoListCard.qml`
    - `src/app/qml/components/InsightPanel.qml`
- Domain pages:
    - `src/app/qml/view/content/CreativeHubPage.qml`
    - `src/app/qml/view/content/BrandHubPage.qml`
    - `src/app/qml/view/content/KnowledgeHubPage.qml`
    - `src/app/qml/view/content/EditorStudioPage.qml`

## Start Checklist

1. Verify `CMakeLists.txt` and `src/app/CMakeLists.txt` still match the LVRS baseline pattern.
2. Verify QML imports are consistently `import LVRS 1.0 as LV`.
3. Design new UI with LVRS components first.

## Completion Checklist

1. Updated docs reflect source changes and repository policy changes together.
2. Updated QML keeps consistent LVRS imports and component usage.
3. Model/viewmodel/view contracts preserve signal-slot hooks (`signals`+`slots` in C++, `signal` + callable hook
   function in QML views).
4. If the user explicitly requests verification or generated artifacts, reuse `build/` and do not create alternate
   build directories.

## Maintenance Rules

- Prioritize build-system simplicity.
- Keep C++ entrypoints minimal and focused.
- Prefer LVRS `Theme` for design tokens.
- Keep command examples and real output paths synchronized in docs.
- Keep model/viewmodel/view interfaces event-driven: every model, viewmodel, and view must expose at least one signal
  and one slot/hook entrypoint.
- Keep hierarchy wiring type-safe: each hierarchy category must remain bound to its dedicated ViewModel in
  `src/app/viewmodel/hierarchy`.
- Enforce SRP for QML: when one QML file starts combining layout shell, rendering delegates, and interaction logic,
  split it into dedicated sibling modules under the same domain directory and compose them from the parent screen.
- Prefer minimum reusable modules for complex editor surfaces (for example: gutter layer, minimap layer, splitter
  interaction layer) instead of embedding every delegate in one root file.
- Keep quality gates aligned with decomposition work:
    - Run `cmake --build build --target whatson_qmllint -j` after QML refactors.
    - Run `cmake --build build --target whatson_clang_tidy -j` after C++ refactors when `clang-tidy` is installed; if
      unavailable, report the missing tool explicitly in the completion notes.

## Troubleshooting Baseline

- LVRS not found: provide LVRS prefix in `CMAKE_PREFIX_PATH`.
- QML module not found: verify LVRS installation state (`lib/qt6/qml/LVRS/qmldir`).
- Link warnings: defer to `lvrs_configure_qml_app()` first and avoid manual link additions.

## Agent Response and Work Rules

- Use real repository files and logs as evidence instead of assumptions.
- Keep changes small and verifiable.
- In completion reports, include changed files, verification commands, and known limits.
- When no tests were run, say so explicitly and cite the repository policy if that is the reason.
- Use English only inside this project.
- Rework `AGENTS.md` when major codebase changes or new requirements are introduced.
