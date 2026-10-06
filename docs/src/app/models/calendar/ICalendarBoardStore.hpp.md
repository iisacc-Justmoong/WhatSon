# `src/app/calendar/ICalendarBoardStore.hpp`

<a id="role"></a>

## 역할
`ICalendarBoardStore`는 달력 표시 컨트롤러에서 사용되는 공유 달력 보드 계약을 정의합니다.

<a id="contract"></a>

## 계약
- 돌연변이: `addEvent`, `addTask`, `removeEntry`, `setTaskCompleted`
- 쿼리: `entriesForDate`, `countsForDate`
- 신호: `entriesChanged`, `entryAdded`, `entryRemoved`, `entryUpdated`

<a id="notes"></a>

## 메모
- 이제 Viewmodel은 구체적인 `CalendarBoardStore` 구현 대신 이 인터페이스에 의존합니다.
- 인터페이스는 메모리 내 저장소 클래스와 별개로 달력 경로에서 개체 간 통신을 유지합니다.
