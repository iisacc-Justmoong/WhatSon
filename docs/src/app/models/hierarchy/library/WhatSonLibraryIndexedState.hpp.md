# `src/app/models/hierarchy/library/WhatSonLibraryIndexedState.hpp`

<a id="responsibility"></a>

## 책임

`WhatSonLibraryIndexedState`는 정식 인덱싱된 라이브러리 노트 세트와 파생된 `draft` 및 `today` 스마트 버킷을 소유하는 백엔드 상태 개체를 정의합니다.

<a id="public-contract"></a>

## 공공 계약

- `indexFromWshub(...)`: 마운트된 허브를 한 번 인덱싱하고 파생된 모든 버킷을 다시 빌드합니다.
- `applySnapshot(...)`: 이미 계산된 런타임 스냅샷 컬렉션을 허용합니다.
- `setIndexedNotes(...)`: 표준 노트 세트를 교체하고 파생 버킷을 다시 빌드합니다.
- `setSourceWshubPath(...)`: 현재 노트 버킷을 건드리지 않고 소스 허브 경로의 대상을 변경합니다.
- `upsertNote(...)` / `removeNoteById(...)` : 하나의 정규 음표를 업데이트하고 해당 변화를 파생된 `draft` / `today` 버킷으로 전파하며 전체 상태 교체를 강제하지 않습니다.
- `noteById(...)`: 돌연변이 및 프로젝션 공동 작업자에 대한 단일 노트 조회를 노출합니다.
- `collectBookmarkedNotes(...)` : 기존 라이브러리 노트 벡터에서 추가 허브 파싱을 강제하지 않고 북마크 후보를 도출합니다.

<a id="architectural-role"></a>

## 아키텍처 역할

이 헤더는 `LibraryHierarchyController`에서 노트 색인 장부를 이동하고 런타임 로딩, 라이브러리 컨트롤러 및 북마크 컨트롤러에서 사용되는 공유 프로젝션 백엔드를 제공하기 위해 존재합니다.
