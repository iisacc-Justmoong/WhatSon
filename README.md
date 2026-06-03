# WhatSon

WhatSon is an LVRS-based Qt Quick desktop application for creative, branding, information, and knowledge text workflows.

## Structure

- `src/app`: LVRS-based desktop UI application.
- `src/daemon`: background daemon entrypoint.
- `src/extension`: optional extension and trial-support code.
- `test/cpp`: maintained Qt Test regression suite.
- `cmake/root`: grouped root custom targets.
- `scripts`: optional developer automation for host build and smoke flows.

## Build Dependencies

- Qt 6.5+ is the baseline Qt requirement. The local maintained kit currently resolves through the configured Qt prefix.
- LVRS is discovered from `~/.local/LVRS` unless a cache override is provided.
- `iiXml` and `iiHtmlBlock` are required local libraries for app and regression builds. Their CMake package prefixes
  default to `~/.local/iiXml` and `~/.local/iiHtmlBlock`.

## Build

Configure and build through the repository-owned `build/` tree:

```bash
cmake -S . -B build
cmake --build build --target whatson_build_regression -j
```

Run the desktop application:

```bash
cmake --build build --target whatson_run_app
```

The host desktop app artifact is emitted at the build root:

- macOS: `build/WhatSon.app`
- Windows: `build/WhatSon.exe`
- Linux and other non-bundle desktop targets: `build/WhatSon`

## Verification Policy

- `whatson_build_regression` builds maintained product binaries plus the regression test executable inside `build/`.
- `whatson_cpp_regression` runs the runtime C++ Qt Test suite.
- `whatson_regression` is the default combined verification gate.
- `whatson_qmllint` is the maintained QML lint target for UI refactors.
- Python test scripts under `scripts/test_*.py` are intentionally absent; automated regression coverage lives under CTest and `test/cpp`.
- `build/` must remain the only verification build directory.

## Current Application Contract

- The workspace shell is desktop-only and is rooted in `src/app/qml/Main.qml`.
- The shell composes status bar, navigation bar, hierarchy sidebar, list bar, content slot, detail panel, and calendar overlays.
- The content slot uses the constrained contents namespace under `src/app/qml/view/contents`.
- Note body editing, body persistence, raw editor sync, and editor command surfaces remain removed until a new document model contract is introduced.
- Resource viewing stays available through `ImageEditor.qml` when the selected resource entry is an image.
- QML owns view construction and view-local behavior only; durable state, parsing, persistence, scheduling, and mutation policy stay in C++ model/controller layers.

## Architecture Rules

- QML files live under `src/app/qml/**`.
- `src/app/models/**` must not contain QML files.
- Project-local includes should use repository-absolute include roots such as `app/...`, `extension/...`, and `test/...`.
- Each hierarchy domain keeps its dedicated controller while sharing `WhatSonHierarchyModel` for display projection.
- Platform app packaging, launch, and export flows are not part of this repository scope.

## Developer Automation

Optional host automation:

```bash
python3 scripts/build_all.py --tasks host
python3 scripts/runtime_smoke_matrix.py --tasks host
```

These scripts are convenience utilities. Completion gates still use the CMake targets listed above.

## 한국어

- 이 저장소는 LVRS 기반 데스크톱 WhatSon 애플리케이션을 소유한다.
- 검증은 반드시 `build/` 기준으로 수행한다.
- 기본 완료 게이트는 `whatson_regression`이다.
- QML은 뷰 구성과 뷰 로컬 동작만 담당하고, 상태/파싱/저장/동기화 정책은 C++ 계층에 둔다.
