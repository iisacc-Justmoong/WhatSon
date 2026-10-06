# `src/app/models/hierarchy/WhatSonHierarchyIoSupport.hpp`

<a id="responsibility"></a>

## 책임

이 헤더는 이전에 계층 구조 도메인 전체에 복제되었던 파일 시스템 도우미를 중앙 집중화합니다.

- `WhatSon::HubPath`를 통해 허브 상대 및 절대 경로 정규화
- 압축이 풀린 `.wshub` 번들에서 `.wscontents` 디렉터리를 해결합니다.
- 공유 디버그 추적 및 오류 보고를 통해 UTF-8 파일 읽기

<a id="public-helpers"></a>

## 공공 도우미

- `normalizePath(...)`
- `resolveContentsDirectories(...)`
- `readUtf8File(...)`
- `deduplicateStringsPreservingOrder(...)`
- `extractDistinctLabelsFromItems(...)`

이러한 도우미는 헤더 전용이며 `WhatSon::Hierarchy::IoSupport`에 있으므로 모든 계층 구조 도메인은 새로운 런타임 종속성을 추가하지 않고도 동일한 구현을 재사용할 수 있습니다.

<a id="performance-notes"></a>

## 성능 노트

`deduplicateStringsPreservingOrder(...)` 및 `extractDistinctLabelsFromItems(...)`는 루프 내부에서 `QStringList::contains(...)`를 호출하는 이전 패턴을 대체합니다.

새로운 도우미는 삽입 순서를 유지하지만 계층 삭제 및 레이블 추출 경로에서 반복되는 O(n^2) 스캔을 제거하는 `QSet`를 사용하여 표시된 값을 추적합니다.

<a id="error-contract"></a>

## 오류 계약

`resolveContentsDirectories(...)` 및 `readUtf8File(...)`는 실패 시 `false`를 반환하고 선택적으로 `errorMessage`를 채웁니다.

여기에서 다루는 실패 사례:

- 널 출력 포인터
- `.wshub` 경로가 비어 있거나 누락되었습니다.
- 잘못된 형식의 압축이 풀린 허브 디렉터리
- `.wscontents` 디렉토리 누락
- 읽을 수 없는 텍스트 파일

<a id="consumers"></a>

## 소비자

이 도우미는 다음에 의해 다시 내보내집니다.

- `LibraryHierarchyControllerSupport.hpp`
- `ProjectsHierarchyControllerSupport.hpp`
- `BookmarksHierarchyControllerSupport.hpp`
- `ProgressHierarchyControllerSupport.hpp`
- `EventHierarchyControllerSupport.hpp`
- `PresetHierarchyControllerSupport.hpp`
- `ResourcesHierarchyControllerSupport.hpp`
- `TagsHierarchyControllerSupport.hpp`

이러한 도메인 헤더는 `using` 선언을 통해 네임스페이스 수준 API를 안정적으로 유지하는 동시에 도메인별 구문 분석 및 직렬화는 각 기능에 대해 로컬로 유지됩니다.
