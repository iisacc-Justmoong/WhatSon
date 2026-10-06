# `src/app/models/panel/NoteActiveStateTracker.cpp`

<a id="responsibility"></a>

## 책임

활성 계층 컨텍스트와 현재 활성 노트 목록 모델을 구독하여 전역 활성 노트 추적을 구현합니다. 선택 및 패키지 경로 게시 시 의도적으로 중지됩니다. 구문 분석된 편집기 세션 마운트, 프로젝션 및 렌더링은 이 개체의 책임이 아닙니다.

<a id="behavior-summary"></a>

## 행동 요약

- `setHierarchyContextSource(...)`는 아키텍처 정책 잠금 전에 사이드바 수준의 `IActiveHierarchyContextSource`를 수락하고, 잠금 후에는 재배선을 거부합니다.
- `synchronizeActiveBindings()`는 활성 계층 인덱스, 활성 계층 컨트롤러 및 활성 메모 목록 모델을 하나의 스냅샷으로 갱신합니다.
- 활성 노트 목록 모델은 다음과 같이 관찰됩니다.
  - `currentIndexChanged()`
  - `currentNoteEntryChanged()`
  - `currentNoteIdChanged()`
  - `currentNoteDirectoryPathChanged()`
  - `noteBackedChanged()`
  - 관련 `QAbstractItemModel` 행/재설정/레이아웃 변경
- 활성 노트 해상도는 `currentNoteEntry`를 선호하고, 그 다음 `currentNoteId/currentNoteDirectoryPath`를 선호하며, 모델에 커밋된 노트 ID 계약이 없을 때만 현재 행 역할 스냅샷으로 되돌아갑니다.
- `bodyText` 행 데이터는 `activeNoteEntry`에서 의도적으로 생략되었습니다; 트래커는 더 이상 본문 텍스트나 본문 파일 경로를 게시하지 않습니다.
- `setActiveNoteState(...)` 다음 엔트리를, 노트 ID 를, 그리고 노트 디렉토리 경로를 다음 변경 신호를 방출하기 전에 기록합니다. 동기식 관찰자는 이전 노트 디렉토리와 새로운 `activeNoteId` 가 짝지어진 것을 결코 볼 수 없습니다.

<a id="tests"></a>

## 테스트

`test/cpp/suites/note_active_state_tracker_tests.cpp` 및 `test/cpp/suites/architecture_policy_lock_tests.cpp`의 아키텍처 잠금 검사에 포함됩니다.

## 한국어

- 대상: `src/app/models/panel/NoteActiveStateTracker.cpp`
- 역할: active hierarchy와 active note-list model의 변화를 구독해 전역 active note 상태를 갱신한다.
- 검증: active hierarchy 전환, 빈 selection clear, `noteBacked=false` clear, architecture lock 이후 재배선 거부를
  회귀 테스트로 고정한다. change signal 중에도 note id와 directory path가 원자적으로 일치하는지도 회귀 테스트로 고정한다.
