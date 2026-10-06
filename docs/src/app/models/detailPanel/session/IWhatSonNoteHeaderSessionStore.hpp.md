# `src/app/models/detailPanel/session/IWhatSonNoteHeaderSessionStore.hpp`

<a id="role"></a>

## 역할
`IWhatSonNoteHeaderSessionStore`는 세션 지원 노트 헤더 편집 계약을 정의합니다.

<a id="contract"></a>

## 계약
- `.wsnhead` 상태에 대한 세션 로드/읽기 액세스입니다.
- 프로젝트, 북마크, 진행 상황, 폴더 및 태그 작업에 대한 헤더 변형.
- 이제 태그 변형에는 인덱싱된 제거 외에도 할당/업서트 경로(`assignTag(...)`)가 명시적으로 포함됩니다.
- 헤더 업데이트를 반영하는 소비자를 위한 `entryChanged(noteId)` 알림입니다.
