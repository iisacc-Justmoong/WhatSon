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

## Codex Init (`/init`) Procedure

Initialization is considered complete when the following sequence succeeds.

1. Confirm LVRS package discoverability.
2. Run CMake configure.
3. Build app and daemon.
4. Run daemon healthcheck.
5. Run optional app offscreen smoke when needed.

Recommended commands:

```bash
cmake -S . -B build
cmake --build build --target WhatSon -j
cmake --build build --target whats_on_daemon -j
./build/src/daemon/whats_on_daemon --healthcheck
./build/src/app/bin/WhatSon.app/Contents/MacOS/WhatSon
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
- Shared components:
    - `src/app/qml/components/NavigationRail.qml`
    - `src/app/qml/components/MetricCard.qml`
    - `src/app/qml/components/InfoListCard.qml`
    - `src/app/qml/components/InsightPanel.qml`
- Domain pages:
    - `src/app/qml/pages/CreativeHubPage.qml`
    - `src/app/qml/pages/BrandHubPage.qml`
    - `src/app/qml/pages/KnowledgeHubPage.qml`
    - `src/app/qml/pages/EditorStudioPage.qml`

## Start Checklist

1. Verify `CMakeLists.txt` and `src/app/CMakeLists.txt` still match the LVRS baseline pattern.
2. Verify QML imports are consistently `import LVRS 1.0 as LV`.
3. Design new UI with LVRS components first.

## Completion Checklist

1. `cmake -S . -B build` passes.
2. `cmake --build build --target WhatSon -j` passes.
3. `cmake --build build --target whats_on_daemon -j` passes.
4. `build/src/daemon/whats_on_daemon --healthcheck` returns `status=ok`.
5. Updated QML keeps consistent LVRS imports and component usage.

## Maintenance Rules

- Prioritize build-system simplicity.
- Keep C++ entrypoints minimal and focused.
- Prefer LVRS `Theme` for design tokens.
- Keep command examples and real output paths synchronized in docs.

## Troubleshooting Baseline

- LVRS not found: provide LVRS prefix in `CMAKE_PREFIX_PATH`.
- QML module not found: verify LVRS installation state (`lib/qt6/qml/LVRS/qmldir`).
- Link warnings: defer to `lvrs_configure_qml_app()` first and avoid manual link additions.

## Agent Response and Work Rules

- Use real repository files and logs as evidence instead of assumptions.
- Keep changes small and verifiable.
- In completion reports, include changed files, verification commands, and known limits.
- Use English only inside this project.
- Rework `AGENTS.md` when major codebase changes or new requirements are introduced.
