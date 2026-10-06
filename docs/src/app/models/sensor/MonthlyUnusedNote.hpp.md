# `src/app/models/sensor/MonthlyUnusedNote.hpp`

<a id="responsibility"></a>

## 책임

`MonthlyUnusedNote`는 최소 한 달 동안 열리지 않은 노트에 대한 고정 창 센서 개체입니다.

<a id="surface"></a>

## 표면

- `WeeklyUnusedNote`를 미러링하여 QML/C++ 호출자가 기간 개체만 교환할 수 있도록 합니다.
- 풍부한 `unusedNotes` 페이로드와 제거된 `unusedNoteIds` 목록을 모두 반환합니다.
