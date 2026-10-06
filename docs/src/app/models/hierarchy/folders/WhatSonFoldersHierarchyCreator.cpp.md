# `src/app/models/hierarchy/folders/WhatSonFoldersHierarchyCreator.cpp`

<a id="responsibility"></a>

## 책임

이 파일은 `WhatSonFolderDepthEntry` 행을 지속형 `Folders.wsfolders` 문서로 다시 직렬화합니다.

<a id="uuid-persistence-rules"></a>

## UUID 지속성 규칙

- 모든 직렬화된 폴더 노드에는 `uuid` 필드가 포함되어 있습니다.
- 호출자가  UUID 를 제공하지 않은 경우, 생성자가 작성하기 전에 하나를 생성합니다.
- 작성자는 다음 사이의 분리를 유지합니다.
  - `id`: 현재 읽을 수 있는 전체 경로
  - `uuid`: 런타임 폴더 관계에서 사용되는 안정적인 ID

<a id="output-invariant"></a>

## 출력 불변

이 작성자가 실행된 후 최신 `.wsfolders` 파일은 두 가지 모두에 대해 충분한 정보를 유지할 것으로 예상됩니다.

- `id`, `label` 및 `depth`에서 표시되는 트리 재구성
- 상위 경로가 변경되는 경우에도 `uuid`를 통해 노트 헤더와 선택 상태를 다시 연결

<a id="main-collaborators"></a>

## 주요 협력자

- `WhatSonFoldersHierarchyStore.cpp`: 저장하기 전에 행을 삭제합니다.
- `WhatSonHubParser.cpp`: 런타임 부트스트랩 중에 폴더 항목을 노출합니다.
- `LibraryHierarchyController.cpp`: 이 작성자를 통해 편집된 계층 구조 행을 유지합니다.
