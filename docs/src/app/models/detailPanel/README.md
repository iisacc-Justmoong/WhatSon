# `src/app/models/detailPanel`

## Status
- Directory mirror generated from the current `src` tree.
- This file is the entry point for the detailed documentation pass of this directory.

## Scope
- Mirrored source directory: `src/app/models/detailPanel`
- Child directories: 1
- Child files: 23

## Child Directories
- `session`

## Child Files
- `DetailContentSectionController.cpp`
- `DetailContentSectionController.hpp`
- `DetailPanelCurrentHierarchyBinder.cpp`
- `DetailPanelCurrentHierarchyBinder.hpp`
- `DetailCurrentNoteContextBridge.cpp`
- `DetailCurrentNoteContextBridge.hpp`
- `DetailFileStatController.cpp`
- `DetailFileStatController.hpp`
- `DetailHierarchySelectionController.cpp`
- `DetailHierarchySelectionController.hpp`
- `DetailNoteHeaderSelectionSourceController.cpp`
- `DetailNoteHeaderSelectionSourceController.hpp`
- `DetailPanelState.cpp`
- `DetailPanelState.hpp`
- `DetailPanelToolbarItemsFactory.cpp`
- `DetailPanelToolbarItemsFactory.hpp`
- `DetailPanelController.cpp`
- `DetailPanelController.hpp`
- `NoteDetailPanelController.hpp`
- `ResourceDetailPanelController.cpp`
- `ResourceDetailPanelController.hpp`
- `DetailPropertiesController.cpp`
- `DetailPropertiesController.hpp`

## Intended Detailed Sections
- Module responsibilities and architectural layer
- Internal submodule boundaries
- Cross-directory dependencies
- Runtime ownership and lifecycle rules
- Testing strategy and coverage map
- Known hotspots and refactor priorities

## Current Notes
- `DetailPanelCurrentHierarchyBinder` now owns the composition-root binding between the active sidebar hierarchy
  context plus the concrete note/resource detail-panel controllers.
- The runtime now mounts a dedicated `NoteDetailPanelController` for note-backed hierarchies and a separate
  `ResourceDetailPanelController` for the resources hierarchy.
- `DetailPanelController` therefore remains the reusable note-detail implementation instead of acting as a shared
  one-size-fits-all detail-panel selector object.

## 한국어

이 섹션은 위 README 내용을 한국어로 확인하기 위한 하단 요약이다.

- 대상: ``src/app/models/detailPanel`` (`docs/src/app/models/detailPanel/README.md`)
- 위치: `docs/src/app/models/detailPanel`
- 역할: 이 파일은 해당 디렉터리나 모듈의 구조, 책임, 운영 규칙, 검증 기준을 설명한다.
- 기준: 파일 경로, 명령, API 이름, 세부 변경 이력은 위 영어 본문을 원문 기준으로 유지한다.
- 변경 시: 위 영어 본문을 수정하면 이 한국어 하단 섹션도 함께 최신 상태로 맞춘다.
