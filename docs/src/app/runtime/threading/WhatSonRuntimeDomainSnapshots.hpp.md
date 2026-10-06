# `src/app/runtime/threading/WhatSonRuntimeDomainSnapshots.hpp`

<a id="responsibility"></a>

## 책임

`WhatSonRuntimeDomainSnapshots.hpp`는 런타임 부트스트랩 및 허브 다시 로드 중에 사용되는 작업자 스레드 페이로드 유형을 선언합니다.

<a id="notable-api"></a>

## 주목할만한 API

- `loadLibrary(...)` : 라이브러리 도메인을 색인하고 `LibraryHierarchyController`가 필요로 하는 노트 레코드, 스마트 버킷 및 파싱된 폴더 계층 구조를 반환합니다.
- `buildBookmarks(...)`: 이미 색인화된 라이브러리 노트 세트에서 북마크 스냅샷을 파생합니다.
- `loadBookmarks(...)` : 대체 경로 경로는 라이브러리 도메인 없이 북마크 도메인이 요청될 때만 사용됩니다.
- `loadHubRuntime(...)`: 이제 전용 `HubRuntimeSnapshot`를 반환하며, 작업 스레드에서 라이브 런타임 스토어를 변형하는 대신 전체 로드된 `WhatSonHubRuntimeStore` 복사본을 스테이저합니다.

<a id="architectural-note"></a>

## 건축 노트

이제 헤더는 북마크에 대한 "index once, derive again" 규칙을 인코딩합니다. `buildBookmarks(...)`가 존재하므로 `WhatSonRuntimeParallelLoader`는 두 번째 허브 통과를 강제하는 대신 공유 라이브러리 스냅샷을 재사용할 수 있습니다.
