# `src/app/models/hierarchy/bookmarks/BookmarksNoteListModel.cpp`

<a id="responsibility"></a>

## 책임

이 구현은 북마크 도메인에 대한 라이브러리 노트 목록 모델 동작을 미러링합니다. 즉, 들어오는 행을 삭제하고, 대체 경로 검색 가능한 텍스트를 파생하고, 검색 텍스트로 필터링하고, 노트 ID별로 선택 항목을 유지하고, 표시되는 소스 캐시를 최신 수정 시간을 기준으로 먼저 정렬합니다.

이제 검색 가능한 텍스트 정규화 후에 저장된 `bodyText`도 지워서 북마크 행이 메모리에 전체 노트 본문을 유지하지 않습니다. 대신 선택한 노트 본문은 편집기 선택 브리지에 의해 천천히 열립니다.

<a id="sorting-pipeline"></a>

## 파이프라인 정렬

북마크 노트 목록은 라이브러리 노트 목록과 동일한 안정적인 내림차순을 사용합니다.

1. `lastModifiedAt`
2. `createdAt`
3. 동점의 원래 상대 순서

이는 가장 최근에 수정된 메모를 계속 승격시키면서 책갈피 목록을 결정적으로 유지합니다.

이제 `setItems(...)`는 들어오는 북마크 페이로드가 현재 페이로드와 동일할 때 정규화된 소스 캐시를 교체하기 전에 단락되므로 중복된 북마크 메모 새로 고침이 다른 재설정/선택 재생 주기를 트리거하지 않습니다.

따라서 `currentBodyText`는 호환성 속성으로만 유지됩니다. 일반 북마크 행은 이제 빈 본문 페이로드를 보고합니다.

<a id="source-metadata"></a>

## 소스 메타데이터
- 소스 경로: `src/app/models/hierarchy/bookmarks/BookmarksNoteListModel.cpp`
- 소스 종류: C++ 구현
- 파일 이름: `BookmarksNoteListModel.cpp`
- 대략적인 줄 수: 631

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

런타임 계층 구조/목록 상호 작용 검사를 통해 북마크 노트 목록 순서 및 투영 동작을 검증합니다.

또한 매우 큰 북마크 메모 선택이 더 이상 해당 메모의 전체 본문 텍스트를 메모리에 보관하는 북마크 목록에 의존하지 않는지 확인하세요.
