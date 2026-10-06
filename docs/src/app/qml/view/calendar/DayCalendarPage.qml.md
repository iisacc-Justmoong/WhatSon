# `src/app/qml/view/calendar/DayCalendarPage.qml`

<a id="role"></a>

## 역할
`DayCalendarPage.qml`는 캘린더 앱에서 흔히 볼 수 있는 24시간 타임라인 패턴을 사용하여 요일 캘린더 오버레이를 렌더링합니다.

<a id="view-contract"></a>

## 계약 보기
- 입력: `dayCalendarController`
- 출력 신호: `noteOpenRequested(string noteId)`
- 후크 신호: `viewHookRequested(string reason)`
- 후크 전달자: `requestViewHook(reason)`는 `dayCalendarController.requestDayView(reason)`에 위임합니다.

<a id="ui-composition"></a>

## UI 구성
- 표면:
  - 루트 페이지는 `LV.Theme.accentTransparent`를 유지하므로 앱 배경이 표시됩니다.
- 헤더:
  - 공유 `CalendarTodayControl`(`Prev/Today/Next`)만 해당됩니다.
- 본체:
  - 나머지 `ContentsView` 높이를 채우는 적응형 24시간 슬롯 타임라인,
  - 왼쪽의 시간 열(`HH:mm`),
  - 공유 `CalendarEventCell`를 사용하여 오른쪽에 시간별로 그룹화된 입장 카드,
  - 슬롯 높이는 `(remainingHeight - spacing) / 24`로 계산되므로 `00:00`는 맨 위 행에서 시작하고 마지막 슬롯은 맨 아래 가장자리에 도달합니다.

<a id="interaction-flow"></a>

## 상호작용 흐름
1. `Component.onCompleted`는 `page-open`를 요청합니다.
2. 헤더 제어 작업은 날짜 커서(`shiftDay`, `setDisplayedDateIso`) 및 요청 후크를 변경합니다.
3. 타임라인은 `DayCalendarController`에서 `timeSlots`에 직접 바인딩됩니다.
4. 투사된 노트 칩을 탭하면 해당 칩의 `sourceId`가 해결되고 `noteOpenRequested(noteId)`가 방출되어 호스트가 편집기 표면에서 해당 노트를 다시 열 수 있습니다.

<a id="tests"></a>

## 테스트
- 현재 이 저장소에는 자동화된 테스트 파일이 없습니다.
- 회귀 체크리스트:
    - 데이뷰 메모 칩은 올바른 투영 슬롯에 계속 표시되어야 합니다.
    - 일별 보기 노트 칩을 클릭하거나 탭하면 `noteOpenRequested(...)`가 방출되어야 합니다.
    - 수동 이벤트 칩은 표시 전용으로 유지되어야 하며 라이브러리 노트를 여는 척해서는 안 됩니다.

<a id="collaborators"></a>

## 협력자
- `src/app/models/calendar/DayCalendarController.hpp/.cpp`
- `src/app/qml/view/calendar/CalendarTodayControl.qml`
- `src/app/qml/view/calendar/CalendarEventCell.qml`
- `src/app/qml/view/panels/ContentViewLayout.qml`
