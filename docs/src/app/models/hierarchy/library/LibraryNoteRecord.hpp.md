# `src/app/models/hierarchy/library/LibraryNoteRecord.hpp`

<a id="responsibility"></a>

## 책임

`LibraryNoteRecord`는 라이브러리 도메인의 한 노트에 대한 정규화된 런타임 페이로드입니다. 노트 목록 모델, 계층 구조 필터 및 유지된 런타임 프로젝션 코드에서 사용되는 공유 데이터 형태입니다.

<a id="relevant-folder-fields"></a>

## 관련 폴더 필드

- `folders`: 프레젠테이션 및 이전 버전과의 호환성에 사용되는 표시되는 폴더 경로입니다.
- `folderUuids`: `folders`를 사용하여 인덱스로 정렬된 안정적인 폴더 ID입니다.

2 목록은 의도적으로 함께 저장되어 애플리케이션이 필터링 및 변형 논리가 안정적 UUID 에 의존하는 동안 읽을 수 있는 경로를 유지할 수 있습니다.

<a id="invariants"></a>

## 불변성

- 필터링이나 지속성을 위해 레코드를 사용하기 전에 `folderUuids`를 정규화해야 합니다.
- 발신자는 `folders` 및 `folderUuids`를 순서대로 정렬하고 개수를 계산해야 합니다.
- 누락된 UUID는 가져오기 중 레거시 폴백으로만 허용되며, 최신 쓰기는 UUID를 보존해야 합니다.
- 레코드에는 노트 본문 소스 또는 파생된 본문 미리보기 필드가 없습니다.
- `progress == -1`는 `No progress`를 나타내며 신규/지워진 노트에 대한 중립 기본값입니다.
- 구조적 동등성이 이제 전체 레코드 페이로드를 포괄하므로, 증분 인덱스 레이어는 무연산 업서트를 억제하고 실제로 노트 레코드가 변경되지 않은 경우 넓은 재구축 신호를 방출하는 것을 방지할 수 있습니다.

<a id="main-collaborators"></a>

## 주요 협력자

- `LibraryAll.cpp`: 현재 라이브러리 색인 경계를 소유합니다.
- `LibraryHierarchyController.cpp`: 선택한 폴더 UUID를 기준으로 레코드를 필터링합니다.
