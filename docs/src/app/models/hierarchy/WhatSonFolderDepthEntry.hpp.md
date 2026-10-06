# `src/app/models/hierarchy/WhatSonFolderDepthEntry.hpp`

<a id="responsibility"></a>

## 책임

`WhatSonFolderDepthEntry`는 하나의 지속형 폴더 행을 간략하게 표현한 메모리 내입니다. 폴더 파서, 폴더 직렬 변환기, 허브 부트스트랩 코드 및 라이브러리 계층 컨트롤러에서 사용됩니다.

<a id="data-contract"></a>

## 데이터 계약

- `id` : `Research/Competitor`와 같은 레거시 전체 경로 식별자. 현재 코드는 `.wsfolders` 데이터가 지속되고 일부 대체 경로 조회가 경로 기반이기 때문에 여전히 이 필드를 유지합니다.
- `label`: 폴더 세그먼트의 표시 이름입니다.
- `depth`: 트리 재구성에 사용되는 0기반 중첩 깊이.
- `uuid`: 영문자와 숫자로 구성된 고정 64자리 폴더 식별자이다. UUID 마이그레이션 이후 런타임 폴더 관계의 기준 식별자이다.

<a id="why-both-id-and-uuid-exist"></a>

## `id`와 `uuid`가 모두 존재하는 이유

저장소는 호환성 단계에 있습니다.

- `.wsfolders`는 여전히 사람이 읽을 수 있는 경로 정보를 저장합니다.
- 노트 헤더와 런타임 선택 로직은 이제 `uuid`를 선호합니다.
- `uuid`가 없는 오래된 데이터는 로드/저장 중에 파서 및 저장에 의해 업그레이드됩니다.

이 분할을 통해 이름 바꾸기 및 상위 재지정 작업을 통해 메모-폴더 바인딩을 무효화하지 않고 표시되는 경로를 변경할 수 있습니다.

<a id="main-collaborators"></a>

## 주요 협력자

- `WhatSonFoldersHierarchyParser.cpp`: `.wsfolders`에서 행을 읽습니다.
- `WhatSonFoldersHierarchyCreator.cpp`: 행을 디스크에 다시 씁니다.
- `LibraryHierarchyController.cpp`: 이러한 행을 UI 방향 계층 구조 항목에 투영합니다.
- `WhatSonLibraryFolderHierarchyMutationService.cpp` : 폴더 경로가 변경될 때 메모 헤더를 정렬하기 위해 UUID를 사용합니다.
