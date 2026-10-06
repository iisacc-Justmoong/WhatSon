# `src/app/models/hierarchy/library/LibraryNoteListModel.hpp`

<a id="responsibility"></a>

## 책임

`LibraryNoteListModel` 는 라이브러리 노트 목록 뷰에 공급하는 구체적 목록 모델 계약입니다. 이는  UI 대상 역할만 노출하며,  `noteId` ,  `primaryText` ,  `bodyText` ,  `displayDate` , 폴더, 태그, 북마크 상태 및  `noteDirectoryPath` 와 같은 역할을 포함하며, 각  `LibraryNoteListItem` 는 이제 내부  `createdAt` 와  `lastModifiedAt` 타임스탬프도 포함합니다.

해당 타임스탬프는 QML 역할로 내보내지지 않습니다. 이는 공개 위임 계약의 나머지 부분을 유지하면서 모델이 가장 최근에 수정된 노트를 기준으로 정렬된 눈에 보이는 노트 행을 유지할 수 있도록 존재합니다.

<a id="ordering-contract"></a>

## 주문계약

- `lastModifiedAt`는 기본 주문 키입니다.
- `createdAt`는 `lastModifiedAt`가 비어 있거나 유효하지 않은 경우 대체 경로 주문 키입니다.
- 유효한 타임스탬프가 없는 항목은 타임스탬프가 있는 항목 뒤에 원래의 상대 순서로 유지됩니다.

이는 보이는 행을 표시할 때 라이브러리 노트 목록이 더 이상 인덱스 파일 추가 순서 또는 생성 시 삽입 순서에 의존하지 않는다는 것을 의미합니다.

선택한 행 계약에는 `currentNoteDirectoryPath`도 포함됩니다. 따라서 보기/컨트롤러 레이어는 `noteId`만 선택된 대상 ID로 처리하지 않고도 중복된 노트 ID를 명확하게 할 수 있습니다.

<a id="source-metadata"></a>

## 소스 메타데이터
- 소스 경로: `src/app/models/hierarchy/library/LibraryNoteListModel.hpp`
- 소스 종류: C++ 헤더
- 파일 이름: `LibraryNoteListModel.hpp`
- 대략적인 줄 수: 116

<a id="extracted-symbols"></a>

## 추출된 기호
- 선언된 네임스페이스 존재: 아니요
- QObject 매크로 존재: 예

<a id="classes-and-structs"></a>

### 클래스와 구조체
- `LibraryNoteListItem`
- `LibraryNoteListModel`

<a id="enums"></a>

### 열거형
- `Role`

<a id="runtime-notes"></a>

## 런타임 노트

- QML 표면은 인덱스 기반이기 때문에 선택 항목은 여전히 가시 행 인덱스에 의해 노출됩니다.
- 새로 고침 시간 선택 복구는 구현에서 노트 ID에 의해 수행되므로, 저장 후에 의존하면 행이 이동하더라도 동일한 논리 노트가 선택된 상태를 유지합니다.
- 내보낸 행 계약은 이제 `noteDirectoryPath` / `currentNoteDirectoryPath`도 표시되므로, 하위 소비 측 선택 코드는 중복된 노트 ID를 구분할 수 있습니다.
