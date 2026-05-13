# `src/app/models/hierarchy`

## Status
- Dedicated model shard for hierarchy models, controllers, parsers, stores, and hierarchy-specific support helpers.
- This directory was promoted from `src/app/models/hierarchy` so hierarchy composition no longer sits under the
  persistent file-storage domain.

## Scope
- Mirrored source directory: `src/app/models/hierarchy`
- Child directories: 9
- Child files: 11

## Child Directories
- `bookmarks`
- `event`
- `folders`
- `library`
- `preset`
- `progress`
- `projects`
- `resources`
- `tags`

## Child Files
- `CMakeLists.txt`
- `IHierarchyCapabilities.hpp`
- `IHierarchyController.cpp`
- `IHierarchyController.hpp`
- `WhatSonFolderDepthEntry.hpp`
- `WhatSonFolderIdentity.hpp`
- `WhatSonHierarchyIoSupport.hpp`
- `WhatSonHierarchyNoteRecordSupport.cpp`
- `WhatSonHierarchyNoteRecordSupport.hpp`
- `WhatSonHierarchyTreeItemSupport.hpp`
- `WhatSonNamedStringHierarchySupport.hpp`

## Intended Detailed Sections
- Module responsibilities and architectural layer
- Internal submodule boundaries
- Cross-directory dependencies
- Runtime ownership and lifecycle rules
- Testing strategy and coverage map
- Known hotspots and refactor priorities

## Shared Expansion Policy
- Concrete hierarchy controllers keep their domain-specific classes and stores, but their right-chevron
  expand/collapse mutation should delegate the common validation/state flip to `IHierarchyController`'s protected
  `setHierarchyItemExpanded(...)` helper.
- Bulk expand/collapse implementations should use `setAllHierarchyItemsExpanded(...)` when a domain exposes a dedicated
  bulk method. Domain code should only perform the follow-up sync/persistence callback.

## 한국어

이 섹션은 위 README 내용을 한국어로 확인하기 위한 하단 요약이다.

- 대상: ``src/app/models/hierarchy`` (`docs/src/app/models/hierarchy/README.md`)
- 위치: `docs/src/app/models/hierarchy`
- 역할: 이 파일은 독립 hierarchy model shard의 구조, 책임, 운영 규칙, 검증 기준을 설명한다.
- 최신 책임: right-chevron expand/collapse의 공통 validation/state flip은 `IHierarchyController` protected helper가
  소유하고, 각 도메인 Controller는 sync/persistence 후처리만 붙인다.
- 기준: 파일 경로, 명령, API 이름, 세부 변경 이력은 위 영어 본문을 원문 기준으로 유지한다.
- 현재: hierarchy 구현은 `src/app/models/hierarchy`가 아니라 `src/app/models/hierarchy`에 둔다.
- 변경 시: 위 영어 본문을 수정하면 이 한국어 하단 섹션도 함께 최신 상태로 맞춘다.
