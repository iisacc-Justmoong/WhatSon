# `src/app/models/hierarchy/library/WhatSonLibraryIndexedState.cpp`

<a id="implementation-summary"></a>

## 구현 요약

구현은  3 하위 레벨 저장 헬퍼를 감쌉니다.

- `LibraryAll`
- `LibraryDraft`
- `LibraryToday`

하나의 백엔드 API 뒤에 이러한 도우미를 유지하므로 상위 계층에서 라이브러리 노트 인덱싱을 단일 책임으로 처리할 수 있습니다.

<a id="derived-bucket-policy"></a>

## 파생 버킷 정책

`rebuildDerivedBuckets()`는 정식 `all` 노트 컬렉션에서 `draft` 및 `today`를 다시 계산하는 내부 경계입니다. 메모를 변경하는 호출자는 표준 메모 세트를 한 번만 교체하면 됩니다.

이제 구현에서는 증분 변형도 지원합니다.

- `setSourceWshubPath(...)`는 현재 메모를 바꾸지 않고 표준 허브 ID를 다시 대상으로 지정합니다.
- `upsertNote(...)`는 하나의 메모에 대해 `LibraryAll`, `LibraryDraft` 및 `LibraryToday`를 업데이트합니다.
- `removeNoteById(...)` 는 전체 3 버킷에서 하나의 노트를 제거합니다

<a id="shared-reuse"></a>

## 공유 재사용

`collectBookmarkedNotes(...)`는 `WhatSonRuntimeDomainSnapshots` 및 `BookmarksHierarchyController`에서 사용하는 일치하는 북마크 프로젝션 도우미를 제공합니다. 이는 허브를 다시 구문 분석하는 대신 이미 인덱싱된 라이브러리 데이터에 대한 북마크 파생을 유지합니다.
