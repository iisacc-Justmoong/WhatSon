# `src/app/models/hierarchy/library/LibraryAll.hpp`

<a id="responsibility"></a>

## 책임

`LibraryAll`는 라이브러리 도메인에 대한 정식 메모리 내 `all notes` 버킷을 소유합니다.

<a id="public-contract"></a>

## 공공 계약

- `indexFromWshub(...)`: 마운트된 허브를 구문 분석하고 표준 노트 벡터를 구축합니다.
- `setIndexedNotes(...)`: 런타임 스냅샷 또는 돌연변이 결과에서 전체 표준 노트 벡터를 대체합니다.
- `setSourceWshubPath(...)` : 호출자가 현재 노트 세트를 보존하고 향후 영속성이 해결될 위치를 변경하고자 할 때만 소스 허브 아이덴티티를 업데이트합니다.
- `upsertNote(...)`: 하나의 메모를 제자리에 삽입하거나 업데이트하고 구조적 무작동 업데이트를 위해 `false`를 반환합니다.
- `removeNoteById(...)`: 전체 버킷을 교체하지 않고 하나의 노트를 정리합니다.
- `noteById(...)`: 돌연변이 또는 프로젝션 협력자에 대한 하나의 노트 레코드를 해결합니다.

<a id="tests"></a>

## 테스트
- 현재 이 저장소에는 자동화된 테스트 파일이 없습니다.
- 회귀 체크리스트:
  - 하나의 음표 본문/메타데이터를 업데이트하는 것은 전체 정규 음표 벡터를 재구성하지 않고 `upsertNote(...)`를 통해 표현 가능해야 합니다.
  - 하나의 노트 삭제는 `removeNoteById(...)`를 통해 표현할 수 있어야 합니다.
  - 무작동 upsert는 `false`를 반환해야 상위 계층이 불필요한 메모 목록/캘린더 재구성을 억제할 수 있습니다.

<a id="architectural-role"></a>

## 아키텍처 역할
이 헤더는 `WhatSonLibraryIndexedState`가 모든 로컬 노트 편집에서 전체 스냅샷 대체를 방지하면서 표준 라이브러리 노트 세트를 동기화된 상태로 유지하는 데 사용하는 안정적인 변형/쿼리 표면을 정의합니다.
