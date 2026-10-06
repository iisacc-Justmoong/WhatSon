# `src/app/models/hierarchy/resources/ResourcesHierarchyControllerSupport.hpp`

<a id="responsibility"></a>

## 책임

이 헤더는 리소스 사이드바에서 사용하는 리소스별 메타데이터 구체화 및 평면화된 `type -> format` 계층 구조 프로젝션을 소유합니다.

<a id="shared-io-delegation"></a>

## 공유 IO 위임

`ResourcesSupport`는 이제 `WhatSonHierarchyIoSupport.hpp`에서 공유 허브 IO를 다시 내보냅니다.

- `normalizePath(...)`
- `resolveContentsDirectories(...)`
- `readUtf8File(...)`
- `deduplicateStringsPreservingOrder(...)`

이렇게 하면 리소스 도메인에서 `.wshub` 순회 및 UTF-8 파일 로드 논리의 또 다른 복사본이 제거됩니다.

<a id="domain-logic-that-stays-local"></a>

## 로컬에 유지되는 도메인 로직

리소스 지원 헤더는 여전히 리소스 패키지에 고유한 논리를 소유합니다.

- 리소스 경로 정리
- 메타데이터 구문 분석 및 대체 경로 구체화
- 구체화된 리소스에서 안정적인 계층 구조 행으로 변환
- 무작동 재설정 감지를 위한 동등성 도우미

`sanitizeStringList(...)` 및 `extractResourcePathsFromItems(...)`는 이제 `QSet` 지원 중복 추적을 사용하므로 가져온 리소스 목록이 커서 반복적인 `QStringList::contains(...)` 스캔을 피할 수 있습니다.

<a id="materialization-rule"></a>

## 구체화 규칙

각 `resourcePath`는 먼저 `WhatSonResourcePackageSupport.hpp`를 통해 확인됩니다.

- 패키지 메타데이터가 유효하면 직접 사용됩니다.
- 그렇지 않으면 대체 경로 메타데이터가 레거시 원시 경로에서 합성됩니다.

이 단계에서는 `resourceId`, `bucket`, `type`, `format` 및 `assetPath`를 정규화합니다.

<a id="stable-keys"></a>

## 안정적인 키

행은 명시적 키를 유지하므로 확장 상태를 안전하게 복원할 수 있습니다. 일반적인 예:

- `type:image`
- `format:image:.png`

이러한 키는 `ResourcesHierarchyController` 확장-복원 논리에 대한 지속성 앵커입니다. 형식 키는 정규화된 조회 형식을 사용하므로 `.PNG` 및 `.png`는 동일한 형식 노드로 축소됩니다.

<a id="empty-state-fallback"></a>

## 빈 상태 대체 경로

`buildHierarchyItems(...)`는 항상 유형 상위 트리를 렌더링하고 기본 형식 카탈로그를 각 유형에 연결합니다. 가져온 리소스 메타데이터는 추가 형식으로 이 카탈로그를 확장하지만 `resourcePaths`가 비어 있는 경우에도 기본 형식 목록이 표시됩니다.

- `Image`
- `Video`
- `Document`
- `3D Model`
- `Web page`
- `Audio`
- `ZIP`
- `Other`

이 타입 행은 확장 가능 (`kind="type"`) 하며 각 행은 형식 자식 (`kind="format"`) 으로 확장되므로 사이드바는 이전 계층 구조 UI 에서 유래한 레거시 `type parent -> format children` 상호작용을 유지합니다. `.mp3`, `.m4a`, `.flac` 와 같은 음악 파일 확장자는 표준 `Audio` 타입 행의 일부입니다. 레거시 `music` / `Music` 패키지 메타데이터는 계층 구조 행이 구축되기 전에 `audio` / `Audio` 로 정규화됩니다.

<a id="equality-contract"></a>

## 평등 계약

`hierarchyItemsEqual(...)`는 QML 렌더링에 사용되는 모든 구조 필드를 비교합니다.

- 깊이와 라벨
- 확장 및 갈매기형 플래그
- 키, 종류, 버킷, 유형, 형식
- 리소스 ID 및 확인된 경로

`ResourcesHierarchyController::setResourcePaths(...)`는 재구성된 계층 구조가 이전 계층 구조와 동일할 때 이 비교기를 사용하여 무작동 모델 재설정을 건너뜁니다.

<a id="maintenance-rule"></a>

## 유지 관리 규칙

`WhatSon::Hierarchy::IoSupport`에서 공유 파일 시스템 동작을 유지합니다. 리소스 도메인 구문 분석 및 계층 구조 형성만 이 헤더에 있어야 합니다.
