# `src/app/models/hierarchy/resources`

## Status
- Directory mirror generated from the current `src` tree.
- This file is the entry point for the detailed documentation pass of this directory.

## Scope
- Mirrored source directory: `src/app/models/hierarchy/resources`
- Child directories: 0
- Child files: 6

## Child Directories
- No child directories.

## Child Files
- `ResourcesHierarchyController.cpp`
- `ResourcesHierarchyController.hpp`
- `ResourcesHierarchyControllerSupport.hpp`
- `ResourcesHierarchyModel.hpp`
- `ResourcesListModel.cpp`
- `ResourcesListModel.hpp`
- `WhatSonResourcePackageSupport.hpp`
- `WhatSonResourcesHierarchyCreator.cpp`
- `WhatSonResourcesHierarchyCreator.hpp`
- `WhatSonResourcesHierarchyParser.cpp`
- `WhatSonResourcesHierarchyParser.hpp`
- `WhatSonResourcesHierarchyStore.cpp`
- `WhatSonResourcesHierarchyStore.hpp`

## Intended Detailed Sections
- Module responsibilities and architectural layer
- Internal submodule boundaries
- Cross-directory dependencies
- Runtime ownership and lifecycle rules
- Testing strategy and coverage map
- Known hotspots and refactor priorities

## Current Notes

- The singular `.wsresource` package contract now includes three package-local artifacts:
  - the original imported asset
  - `resource.xml`
  - `annotation.png` as a transparent bitmap canvas reserved for future resource-editor sketch/annotation overlay work
- `WhatSonResourcePackageSupport.hpp` owns both sides of that package contract:
  - metadata XML creation/parsing for the new `annotationPath`
  - empty annotation bitmap generation/writing for package creation paths
- `ResourcesHierarchyModel.hpp` only declares the resources hierarchy item struct and icon helper. The displayed
  hierarchy item model is the shared `WhatSonHierarchyModel`, so resource chevron changes update one expanded row
  instead of resetting a domain-specific model.

## 한국어

이 섹션은 위 README 내용을 한국어로 확인하기 위한 하단 요약이다.

- 대상: ``src/app/models/hierarchy/resources`` (`docs/src/app/models/hierarchy/resources/README.md`)
- 위치: `docs/src/app/models/hierarchy/resources`
- 역할: 이 파일은 해당 디렉터리나 모듈의 구조, 책임, 운영 규칙, 검증 기준을 설명한다.
- 현재 규칙: 리소스 hierarchy도 표시용 item model은 공통 `WhatSonHierarchyModel`을 사용하고, domain struct는
  `ResourcesHierarchyModel.hpp`에만 남긴다.
- 기준: 파일 경로, 명령, API 이름, 세부 변경 이력은 위 영어 본문을 원문 기준으로 유지한다.
- 변경 시: 위 영어 본문을 수정하면 이 한국어 하단 섹션도 함께 최신 상태로 맞춘다.
