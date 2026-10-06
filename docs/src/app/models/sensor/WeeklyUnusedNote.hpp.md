# `src/app/models/sensor/WeeklyUnusedNote.hpp`

<a id="responsibility"></a>

## 책임

`WeeklyUnusedNote`는 최소 일주일 동안 열리지 않은 노트에 대한 고정 창 센서 개체입니다.

<a id="surface"></a>

## 표면

- `hubPath`: 검사할 `.wshub` 루트의 압축을 풉니다.
- `unusedNotes`: 타임스탬프 및 메모 경로를 포함한 전체 항목 페이로드.
- `unusedNoteIds`: 필터링된 메모 ID를 편리하게 투영합니다.
- `unusedNoteCount`: 바인딩에 대한 카운트 투영입니다.
- `lastError`: 유효성 검사 또는 스캔 실패 텍스트.

<a id="event-model"></a>

## 이벤트 모델

- 필터링된 결과가 변경될 때마다 `unusedNotesChanged()`를 내보냅니다.
- 빈 상태 새로 고침을 포함하여 새로 고칠 때마다 `scanCompleted(...)`를 내보냅니다.
- `refresh()`를 모델 도메인의 필수 슬롯 진입점으로 노출합니다.
