# `src/app/models/hierarchy/bookmarks/BookmarksNoteListModel.hpp`

<a id="responsibility"></a>

## 책임

`BookmarksNoteListModel` 는  `LibraryNoteListModel` 의 북마크 도메인 동료입니다. 동일한 대리자 대상 행 역할을 노출하며, 이제 각  `BookmarksNoteListItem` 에 내부  `createdAt` /  `lastModifiedAt` 필드를 포함하여 북마크 목록이 메인 라이브러리 목록과 동일한 가장 최근 수정 우선 정렬 정책을 따를 수 있습니다.

<a id="ordering-contract"></a>

## 주문계약

- 행은 `lastModifiedAt` 내림차순으로 정렬됩니다.
- `createdAt`는 `lastModifiedAt`가 누락된 경우에 사용됩니다.
- 동점은 들어오는 상대 순서를 유지합니다.

따라서 북마크 사이드바는 북마크된 여러 노트가 함께 표시될 때 허브 구문 분석 순서에 의존하지 않습니다.

<a id="source-metadata"></a>

## 소스 메타데이터
- 소스 경로: `src/app/models/hierarchy/bookmarks/BookmarksNoteListModel.hpp`
- 소스 종류: C++ 헤더
- 파일 이름: `BookmarksNoteListModel.hpp`
- 대략적인 줄 수: 116

<a id="extracted-symbols"></a>

## 추출된 기호
- 선언된 네임스페이스 존재: 아니요
- QObject 매크로 존재: 예

<a id="classes-and-structs"></a>

### 클래스와 구조체
- `BookmarksNoteListItem`
- `BookmarksNoteListModel`

<a id="enums"></a>

### 열거형
- `Role`

<a id="runtime-notes"></a>

## 런타임 노트

선택은 여전히 가시적 행 인덱스로 공개되지만 구현에서는 필터 또는 리조트 작업 후에 노트 ID별로 선택을 복원합니다.
