# `src/app/models/calendar/YearCalendarController.cpp`

<a id="implementation-notes"></a>

## 구현 노트
- 연도 재구축은 이제 `ICalendarBoardStore::entriesChanged`를 관찰합니다.
- `requestYearView(...)`는 이제 훅/로그 전용이며, 실제 연식 모델 재구성은 `setDisplayedYear(...)`, `setCalendarSystemByEnum(...)`, `setCalendarBoardStore(...)` 및 보드 엔트리 변이를 그대로 유지합니다.
