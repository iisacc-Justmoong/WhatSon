# `src/app/models/calendar/MonthCalendarController.hpp`

<a id="role"></a>

## 역할
`MonthCalendarController`는 달력 그리드 월 예측을 공개합니다.

<a id="interface-alignment"></a>

## 인터페이스 정렬
- 이제 캘린더 보드 삽입은 `ICalendarBoardStore`에 따라 달라집니다.
- `monthProjectionFor(year, month)`는 임의의 월 그리드 쿼리에 계속 사용할 수 있습니다.
- 컨트롤러는 이제 표준 `previous/current/next` 월 페이지에 대한 사전 계산된 `pagerMonthModels` 속성을 노출하므로 `MonthCalendarPage.qml` 는 더 이상 QML 내부에서 인접 월 프로젝트를 조립할 필요가 없습니다.
