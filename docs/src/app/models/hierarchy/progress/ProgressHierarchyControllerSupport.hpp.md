# `src/app/models/hierarchy/progress/ProgressHierarchyControllerSupport.hpp`

<a id="responsibility"></a>

## 책임

이 헤더는 진행 상황별 계층 구조 구문 분석, 직렬화 및 삭제 도우미를 소유합니다.

<a id="shared-io-delegation"></a>

## 공유 IO 위임

`ProgressSupport`는 이제 `WhatSonHierarchyIoSupport.hpp`에서 공유 허브 IO를 다시 내보냅니다.

- `normalizePath(...)`
- `resolveContentsDirectories(...)`
- `readUtf8File(...)`
- `deduplicateStringsPreservingOrder(...)`
- `extractDistinctLabelsFromItems(...)`

중복된 인라인 `.wshub` 탐색 및 UTF-8 로딩 로직이 이 파일에서 제거되었습니다.

<a id="shared-tree-delegation"></a>

## 공유 트리 위임

`ProgressSupport`는 또한 `WhatSonHierarchyTreeItemSupport.hpp`에서 공유 트리 돌연변이 도우미를 다시 내보냅니다.

- `applyChevronByDepth(...)`
- `nextGeneratedFolderSequence(...)`
- `renameHierarchyItem(...)`
- `isBucketHeaderItem(...)`
- `deleteHierarchySubtree(...)`

`createHierarchyFolder(...)`는 강제 상위 확장 없이 중첩된 삽입 도우미 주변의 얇은 래퍼로 유지됩니다.

<a id="domain-logic-that-stays-local"></a>

## 로컬에 유지되는 도메인 로직

다음 도우미는 진행 계층 구조 페이로드를 형성하기 때문에 진행 상황별로 유지됩니다.

- `sanitizeStringList(...)`
- `clampSelectionIndex(...)`
- `parseItemEntry(...)`
- `parseDepthItems(...)`
- `serializeDepthItems(...)`
- 헤더의 뒷부분에 정의된 같음 및 빌더 도우미

`sanitizeStringList(...)` 및 `extractDomainLabelsFromItems(...)`는 이제 공유 `QSet` 기반 순서형 중복 제거 도우미를 사용하므로 반복적인 `contains(...)` 스캔으로 인해 대규모 진행 상태 목록이 더 이상 저하되지 않습니다.

<a id="current-build-rules"></a>

## 현재 빌드 규칙

- `sanitizeStringList(...)`는 원시 상태 레이블을 자르고 중복을 제거합니다.
- `buildSupportedTypeItems(...)`는 제품에 정의된 10-행 진행 분류 체계를 반환합니다: `First draft`, `Modified draft`, `In Progress`, `Pending`, `Reviewing`, `Waiting for approval`, `Done`, `Lagacy`, `Archived`, 및 `Delete review`.
- 첫 번째 4 행은 현재 LVRS / Figma 상호작용 계약을 유지하기 위해 의도적으로 `showChevron=true` 를 유지하며, 자식 행이 없어도 됩니다.
- 직렬화된 깊이 항목에는 공유 계층 구조인 QML 레이어에 필요한 행 레이블 및 구조 메타데이터가 포함되며, 컨트롤러는 해당 출력 위에 `progressValue`와 `itemId`를 추가합니다.

<a id="maintenance-rule"></a>

## 유지 관리 규칙

`WhatSon::Hierarchy::IoSupport`에서 공유 파일 시스템 동작을 유지합니다. 이 지원 헤더에는 진행 도메인 규칙만 추가해야 합니다.
