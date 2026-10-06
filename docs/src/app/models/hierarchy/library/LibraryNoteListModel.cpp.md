# `src/app/models/hierarchy/library/LibraryNoteListModel.cpp`

<a id="responsibility"></a>

## 책임

이 구현은 들어오는 노트 목록 항목을 삭제하고, 검색 가능한 텍스트 대체를 파생하고, 검색 필터링을 적용하고, 재설정 시 ID별로 선택한 노트를 보존하고, 이제 라이브러리 노트에 대해 정식 최신-수정-순 정렬을 수행합니다.

런타임 노트 목록 투영은 이제 `bodyText`를 비워 둡니다. 목록 모델은 테스트 또는 편집기가 아닌 호출자가 제공한 명시적 행 페이로드를 계속 전달할 수 있지만 더 이상 편집기 화면에 대해 선택된 노트 본문 계약을 제공하지 않습니다.

`primaryText`는 편집기 RAW 소스가 아닌 표시 가능한 메타데이터입니다.

이제 각 행은 정규화된 `noteDirectoryPath`를 유지하며 선택된 행 계약은 `currentNoteDirectoryPath`를 통해 해당 값을 노출합니다. 따라서 목록 모델은 더 이상 `noteId`만으로 선택한 메모를 설명하지 않습니다. 하위 소비 측 선택 코드는 구체적인 가시 행 ID를 계속 따를 수 있습니다.

<a id="sorting-pipeline"></a>

## 파이프라인 정렬

이제 `setItems(...)`는 필터링된 소스 캐시가 저장되기 전에 `createdAt` 및 `lastModifiedAt`를 정규화합니다.

정리된 소스 캐시가 교체되기 전에 모델은 이제 전체 정규화된 항목 목록을 기존 소스 캐시와 비교하고 실제로 변경된 사항이 없을 때 일찍 반환합니다. 이는 동등한 라이브러리 새로 고침이 또 다른 전체 목록 재설정을 강제하는 것을 방지합니다.

그런 다음 모델은 다음을 사용하여 안정적인 내림차순 정렬을 수행합니다.

1. `lastModifiedAt`
2. `createdAt`
3. 동점 또는 유효하지 않은 타임스탬프에 대한 원래 상대 순서

동일한 타임스탬프를 가진 노트는 새로 고침 사이에 지터가 있어서는 안 되기 때문에 안정적인 정렬 요구 사항이 중요합니다.

<a id="selection-stability"></a>

## 선택 안정성

`applySearchFilter()`는 이전에 선택한 논리 메모를 통해 여전히 `m_currentIndex`를 복원하며, 표시되는 패키지 경로가 변경되면 이제 선택한 행 알림에 `currentNoteDirectoryPathChanged()`도 포함됩니다. 이렇게 하면 표시되는 노트 행에서 중복 ID가 명확하게 유지됩니다.

같은 초기화 경로는 필터링된 선택 항목이 처음 실체화되거나, 선택한 행이 같은 표시 인덱스에서 제자리 교체될 때 `currentNoteEntryChanged()`도 출력한다. 이로써 `currentNoteEntry` 소비자는 `currentIndexChanged()`에만 의존하지 않고 기준 행 선택과 일치한 상태를 유지한다.

`currentBodyText`는 다시 실제 선택 노트 페이로드입니다. 검색/필터를 재설정한 후에도 모델은 여전히 ​​노트 ID별로 `m_currentIndex`를 복원하고, 선택한 행은 `currentBodyText` 및 `BodyTextRole`를 통해 정규화된 노트 본문을 계속 노출합니다.

`applySearchFilter()` 추적은 이제 이동된 임시 필터 버퍼 대신 재설정 후 `m_items` 상태를 기록합니다. 따라서 `nextCount`, `nextItemId` 및 `nextItemDirectoryPath` 필드는 하위 소비 측 선택 디버깅이 검사해야 하는 실제 표시 목록 상태를 설명합니다.

<a id="source-metadata"></a>

## 소스 메타데이터
- 소스 경로: `src/app/models/hierarchy/library/LibraryNoteListModel.cpp`
- 소스 종류: C++ 구현
- 파일 이름: `LibraryNoteListModel.cpp`
- 대략적인 줄 수: 628

<a id="extracted-symbols"></a>

## 추출된 기호
- 선언된 네임스페이스 존재: 아니요
- QObject 매크로 존재: 아니요

<a id="classes-and-structs"></a>

### 클래스와 구조체
- `ValidationIssue`

<a id="enums"></a>

### 열거형
- 스캐폴드 생성 중에 감지된 항목이 없습니다.

<a id="verification"></a>

## 검증

런타임 라이브러리-컨트롤러 동기화 확인을 통해 라이브러리 노트 목록 순서를 확인합니다.

또한  런타임 스냅샷 새로고침 및 검색/필터 초기화 후 선택된 라이브러리 노트가 노트 목록 모델을 통해 즉시 본문 텍스트를 노출하는지 확인하십시오. 중복 노트 ID 가 존재하는 경우, 마운트된 패키지에 대한 올바른  `noteDirectoryPath` 를 선택된 행이 계속 내보내는지도 확인하십시오. 첫 번째 가시 선택이 구체화되거나 선택된 행이 초기화로 교체될 때,  `currentNoteEntryChanged()` 가 새 행 페이로드와 함께 정확히 한 번만 방출되도록 확인하십시오.
