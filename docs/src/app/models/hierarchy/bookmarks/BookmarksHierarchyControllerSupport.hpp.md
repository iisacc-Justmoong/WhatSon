# `src/app/models/hierarchy/bookmarks/BookmarksHierarchyControllerSupport.hpp`

<a id="responsibility"></a>

## 책임

이 헤더는 북마크별 계층 구조 구문 분석, 직렬화 및 삭제 도우미를 소유합니다.

<a id="shared-io-delegation"></a>

## 공유 IO 위임

`BookmarksSupport`는 이제 `WhatSonHierarchyIoSupport.hpp`에서 공유 허브 IO를 다시 내보냅니다.

- `normalizePath(...)`
- `resolveContentsDirectories(...)`
- `readUtf8File(...)`
- `deduplicateStringsPreservingOrder(...)`
- `extractDistinctLabelsFromItems(...)`

중복된 인라인 `.wshub` 탐색 및 UTF-8 로딩 로직이 이 파일에서 제거되었습니다.

<a id="shared-tree-delegation"></a>

## 공유 트리 위임

`BookmarksSupport`는 또한 `WhatSonHierarchyTreeItemSupport.hpp`에서 공유 트리 돌연변이 도우미를 다시 내보냅니다.

- `applyChevronByDepth(...)`
- `nextGeneratedFolderSequence(...)`
- `renameHierarchyItem(...)`
- `isBucketHeaderItem(...)`
- `deleteHierarchySubtree(...)`

`createHierarchyFolder(...)`는 강제 상위 확장 없이 중첩된 삽입 도우미 주변의 얇은 래퍼로 유지됩니다.

<a id="domain-logic-that-stays-local"></a>

## 로컬에 유지되는 도메인 로직

다음 도우미는 북마크 계층 구조 페이로드를 형성하므로 북마크별로 유지됩니다.

- `sanitizeStringList(...)`
- `clampSelectionIndex(...)`
- `parseItemEntry(...)`
- `parseDepthItems(...)`
- `serializeDepthItems(...)`
- 헤더의 뒷부분에 정의된 같음 및 빌더 도우미

`sanitizeStringList(...)` 및 `extractDomainLabelsFromItems(...)`는 이제 공유 `QSet` 기반 정렬된 중복 제거 도우미를 사용하므로 북마크 계층 구조 재구축 시 반복되는 선형 중복 확인을 방지할 수 있습니다.

<a id="maintenance-rule"></a>

## 유지 관리 규칙

`WhatSon::Hierarchy::IoSupport`에서 공유 파일 시스템 동작을 유지합니다. 이 지원 헤더에는 북마크 도메인 규칙만 추가해야 합니다.
