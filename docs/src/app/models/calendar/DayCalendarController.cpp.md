# `src/app/models/calendar/DayCalendarController.cpp`

<a id="implementation-notes"></a>

## 구현 노트
- 주간 모델 재구축은 `ICalendarBoardStore::entriesChanged`를 수신합니다.
- `requestDayView(...)`는 이제 훅/로그 전용이며, 실제 재구성 소유권은 `setDisplayedDateIso(...)`, `setCalendarBoardStore(...)` 및 `entriesChanged`에 남아 있어 `shiftDay(...)` / `jumpToToday()` 이후의 중복 재계산 경로를 제거합니다.
